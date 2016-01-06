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

CMON_DIR="/caramon"

CMON_LOG_FILE=${CMON_DIR}/cmon.log
CMON_TERMINATE_ERROR_FILE=${CMON_DIR}/cmon_terminate_error_cnt

CMON_EXE="cmon_rel.arm"
CMON_EXE_ARGS="-l ${CMON_LOG_FILE} -t ${CMON_TERMINATE_ERROR_FILE}"

MAX_TERMINATE_ERRORS=5

SIG_TERMINATE_CMON="SIGTERM"

GPIO_PIN_START_DISABLED="22" # GPIO22, Pin15, Connector P1

################################################################
function is_start_disabled()
################################################################
{
    # Set pin as input
    echo "${GPIO_PIN_START_DISABLED}" > /sys/class/gpio/export
    echo "in" > /sys/class/gpio/gpio${GPIO_PIN_START_DISABLED}/direction
    
    # Read pin
    pin_value=`cat /sys/class/gpio/gpio${GPIO_PIN_START_DISABLED}/value`

    # Restore pin
    echo "${GPIO_PIN_START_DISABLED}" > /sys/class/gpio/unexport

    echo ${pin_value}
    return 0
}

################################################################
function get_terminate_errors()
################################################################
{
    errors=0

    # If no error file exists, all is well
    if [ ! -f ${CMON_TERMINATE_ERROR_FILE} ]; then
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
    PID=`ps -e | grep ${CMON_EXE} | awk '{print $1}'`
    if [ -z "$PID" ]; then
	echo ""
	return 1
    fi

    echo $PID
    return 0
}

################################################################
function do_start()
################################################################
{
    # Check if start is disabled by GPIO pin
    #start_disabled=`is_start_disabled`
    start_disabled=0

    # Start application
    if [ $start_disabled -eq 1 ]; then
	echo "Start CMON - disabled by GPIO pin ${GPIO_PIN_START_DISABLED}"
	rm -rf ${CMON_TERMINATE_ERROR_FILE}
    else
	# Turn on corefiles with unlimited size
	#ulimit -c unlimited

	# Check if maximum number of terminate errors is reached
	terminate_errors=`get_terminate_errors`
	if [ $terminate_errors -ge ${MAX_TERMINATE_ERRORS} ]; then
	    echo "Start CMON - disabled by max number of terminate errors"
	else
	    echo "Start CMON"
	    ${CMON_DIR}/${CMON_EXE} ${CMON_EXE_ARGS} &
	fi
    fi

    exit 0
}

################################################################
function do_shutdown()
################################################################
{
    PID=`pid_of_cmon`
    if [ $? -ne 0 ]; then
	exit 1
    fi

    echo "Shutdown CMON"
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

    shutdown)
	do_shutdown
        ;;

    *)
        echo "Usage $0 {start|shutdown}"
        exit 1
        ;;
esac
