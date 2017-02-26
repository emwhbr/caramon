#!/bin/bash
# /************************************************************************
#  *                                                                      *
#  * Copyright (C) 2016 Bonden i Nol (hakanbrolin@hotmail.com)            *
#  *                                                                      *
#  * This program is free software; you can redistribute it and/or modify *
#  * it under the terms of the GNU General Public License as published by *
#  * the Free Software Foundation; either version 2 of the License, or    *
#  * (at your option) any later version.                                  *
#  *                                                                      *
#  ************************************************************************/

# Directories and files
CMON_DIR="/caramon"
CMON_LOG_FILE=${CMON_DIR}/cmon.log

CMON_TERMINATE_ERROR_FILE=""
CMON_TERMINATE_SW_WD_ERROR_FILE=${CMON_DIR}/cmon_terminate_sw_wd_error_cnt
CMON_TERMINATE_HW_WD_ERROR_FILE=${CMON_DIR}/cmon_terminate_hw_wd_error_cnt

CMON_START_TIME_FILE=${CMON_DIR}/cmon_start_time
CMON_FALLBACK_FILE=${CMON_DIR}/cmon_fallback

# Executable and arguments
CMON_EXE="cmon_rel.arm"

CMON_EXE_ARGS=""
CMON_SW_WD_EXE_ARGS="-l ${CMON_LOG_FILE} -t ${CMON_TERMINATE_SW_WD_ERROR_FILE} -p"
CMON_HW_WD_EXE_ARGS="-l ${CMON_LOG_FILE} -p -w"
CMON_SW_WD_EXE_ARGS_FALLBACK="-l ${CMON_LOG_FILE} -t ${CMON_TERMINATE_SW_WD_ERROR_FILE} -f"
CMON_HW_WD_EXE_ARGS_FALLBACK="-l ${CMON_LOG_FILE} -f -w"

# Watchdog
HW_WD_ENABLE=1     # 1 :  Use hardware watchdog
                   # 0 :  Use software watchdog (involving cron and cmon_watchdog.sh)

MAX_TERMINATE_ERRORS=""
MAX_TERMINATE_SW_WD_ERRORS=10
MAX_TERMINATE_HW_WD_ERRORS=5

MIN_REQUIRED_HW_WD_RESTART_TIME=300  # Minimum required time in seconds since last start.
                                     # This is the minimum amount of time that the application
                                     # must execute to be considered OK.
# Misc
SIG_TERMINATE_CMON="SIGTERM"
GPIO_PIN_INHIBIT_START="27" # BCM-GPIO-27, Connector P1-13

################################################################
function setup_mode()
################################################################
{
    # $1 : 0   Use normal mode   
    # $1 : 1   Use fallback mode

    if [ ${HW_WD_ENABLE} -eq 1 ]; then
	# Hardware watchdog is enabled
	echo "CMON - setup for HW watchdog, fallback:$1"
	rm -f ${CMON_TERMINATE_SW_WD_ERROR_FILE}
	if [ $1 -eq 0 ]; then
	    CMON_EXE_ARGS=${CMON_HW_WD_EXE_ARGS}
	else
	    CMON_EXE_ARGS=${CMON_HW_WD_EXE_ARGS_FALLBACK}
	fi
	CMON_TERMINATE_ERROR_FILE=${CMON_TERMINATE_HW_WD_ERROR_FILE}
	MAX_TERMINATE_ERRORS=${MAX_TERMINATE_HW_WD_ERRORS}
    else
	# Software watchdog is enabled
	echo "CMON - setup for SW watchdog, fallback:$1"
	rm -f ${CMON_TERMINATE_HW_WD_ERROR_FILE}
	if [ $1 -eq 0 ]; then
	    CMON_EXE_ARGS=${CMON_SW_WD_EXE_ARGS}
	else
	    CMON_EXE_ARGS=${CMON_SW_WD_EXE_ARGS_FALLBACK}
	fi
	CMON_TERMINATE_ERROR_FILE=${CMON_TERMINATE_SW_WD_ERROR_FILE}
	MAX_TERMINATE_ERRORS=${MAX_TERMINATE_SW_WD_ERRORS}
    fi
}

################################################################
function is_start_disabled()
################################################################
{
    local pin_value

    # Set pin as input
    echo "${GPIO_PIN_INHIBIT_START}" > /sys/class/gpio/export
    echo "in" > /sys/class/gpio/gpio${GPIO_PIN_INHIBIT_START}/direction
    
    # Read pin
    pin_value=`cat /sys/class/gpio/gpio${GPIO_PIN_INHIBIT_START}/value`

    # Restore pin
    echo "${GPIO_PIN_INHIBIT_START}" > /sys/class/gpio/unexport

    echo ${pin_value}
    return 0
}

################################################################
function get_terminate_errors()
################################################################
{
    local errors

    # If no error file exists, all is well
    if [ ! -f ${CMON_TERMINATE_ERROR_FILE} ]; then
	echo 0 > ${CMON_TERMINATE_ERROR_FILE}
	errors=0
    else
	read errors <  ${CMON_TERMINATE_ERROR_FILE}
    fi

    echo ${errors}
    return 0
}

################################################################
function pid_of_cmon()
################################################################
{
    local PID

    PID=`ps -e | grep ${CMON_EXE} | awk '{print $1}'`
    if [ -z "$PID" ]; then
	echo ""
	return 1
    fi

    echo $PID
    return 0
}

################################################################
function do_start_hw_wd()
################################################################
{
    local start_time
    local last_start_time
    local delta_time
    local terminate_errors

    # Once in fallback, always fallback.
    # There is no turning back and we must manually fix the problem.
    if [ -f ${CMON_FALLBACK_FILE} ]; then
	echo "CMON - start normal mode disabled by permanent fallback"
	do_fallback
	return 0
    fi

    # Get current start time in seconds since 1970-01-01 00:00:00 UTC
    start_time=`date +%s`

    # Get last start time
    if [ ! -f ${CMON_START_TIME_FILE} ]; then
	last_start_time=0
    else
	read last_start_time < ${CMON_START_TIME_FILE}
    fi

    # Save current start time
    echo ${start_time} > ${CMON_START_TIME_FILE}

    # Check if current start time is too close to the previous one.
    # This probably means that the last start was terminated by the
    # hardware watchdog. But this can also be a power failure ...
    delta_time=$((start_time-last_start_time))
    if [ ${delta_time} -lt ${MIN_REQUIRED_HW_WD_RESTART_TIME} ]; then
	# Previous start was terminated too early
	terminate_errors=`get_terminate_errors`
	terminate_errors=$((terminate_errors+1))
    else
	# Prevoius start was running long enough to be considered OK.
	# Let's reset the error history.
	terminate_errors=0
    fi
    echo $terminate_errors > ${CMON_TERMINATE_ERROR_FILE}

    # Check if maximum number of terminate errors is reached
    terminate_errors=`get_terminate_errors`
    if [ $terminate_errors -ge ${MAX_TERMINATE_ERRORS} ]; then
	echo "CMON - start normal mode disabled by max number of terminate errors"
	do_fallback
    else
	echo "CMON - start normal mode"
	echo "CMON - cmd: ${CMON_DIR}/${CMON_EXE} ${CMON_EXE_ARGS} &"
	# JOE: Commented below for testing
	${CMON_DIR}/${CMON_EXE} ${CMON_EXE_ARGS} &
    fi

    return 0
}

################################################################
function do_start_sw_wd()
################################################################
{
    local terminate_errors

    # Check if maximum number of terminate errors is reached
    terminate_errors=`get_terminate_errors`
    if [ $terminate_errors -ge ${MAX_TERMINATE_ERRORS} ]; then
	echo "CMON - start normal mode disabled by max number of terminate errors"
    else
	echo "CMON - start normal mode"
	echo "CMON - cmd: ${CMON_DIR}/${CMON_EXE} ${CMON_EXE_ARGS} &"
	# JOE: Commented below for testing
	${CMON_DIR}/${CMON_EXE} ${CMON_EXE_ARGS} &
    fi
}

################################################################
function do_start()
################################################################
{
    local start_disabled

    # Check if start is disabled by GPIO pin
    start_disabled=`is_start_disabled`   
    if [ $start_disabled -eq 1 ]; then
	echo "CMON - start disabled by GPIO#${GPIO_PIN_INHIBIT_START} (BCM)"
	rm -f ${CMON_TERMINATE_SW_WD_ERROR_FILE}
	rm -f ${CMON_TERMINATE_HW_WD_ERROR_FILE}
	rm -f ${CMON_START_TIME_FILE}
	rm -f ${CMON_FALLBACK_FILE}
    else
	 # Prepare normal mode
	setup_mode 0

	# Turn on corefiles with unlimited size
	#ulimit -c unlimited

	# Start by using selected watchdog
	if [ ${HW_WD_ENABLE} -eq 1 ]; then
	    do_start_hw_wd  # Hardware watchdog is enabled
	else
	    do_start_sw_wd  # Software watchdog is enabled
	fi
    fi

    exit 0
}

################################################################
function do_fallback()
################################################################
{
    local start_disabled

    # Check if start is disabled by GPIO pin
    start_disabled=`is_start_disabled`
    if [ $start_disabled -eq 1 ]; then
	echo "CMON - start disabled by GPIO#${GPIO_PIN_INHIBIT_START} (BCM)"
	rm -f ${CMON_TERMINATE_SW_WD_ERROR_FILE}
	rm -f ${CMON_TERMINATE_HW_WD_ERROR_FILE}
	rm -f ${CMON_START_TIME_FILE}
	rm -f ${CMON_FALLBACK_FILE}
    else
	# Prepare fallback mode
	setup_mode 1
	touch ${CMON_FALLBACK_FILE}
	
	# Start by using selected watchdog
	echo "CMON - start fallback mode"
	echo "CMON - cmd: ${CMON_DIR}/${CMON_EXE} ${CMON_EXE_ARGS} &"
	# JOE: Commented below for testing
	${CMON_DIR}/${CMON_EXE} ${CMON_EXE_ARG} &
    fi

    exit 0
}

################################################################
function do_shutdown()
################################################################
{
    local PID

    PID=`pid_of_cmon`
    if [ $? -ne 0 ]; then
	exit 1
    fi

    echo "CMON - Shutdown"
    kill -${SIG_TERMINATE_CMON} ${PID}

    exit 0
}

################################################################
#                 MAIN SCRIPT STARTS HERE ...
################################################################

case "$1" in
    start)	
	do_start
        ;;

    fallback)
	do_fallback
	;;

    shutdown)
	do_shutdown
        ;;

    *)
        echo "Usage $0 {start|fallback|shutdown}"
        exit 1
        ;;
esac
