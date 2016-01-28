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

CMON_WATCHDOG_LOG=${CMON_DIR}/cmon_watchdog.log
CMON_TERMINATE_ERROR_FILE=${CMON_DIR}/cmon_terminate_error_cnt

CMON_UTIL="cmon_util.sh"
CMON_EXE="cmon_rel.arm"

MAX_TERMINATE_ERRORS=10

################################################################
function get_terminate_errors()
################################################################
{
    errors=0
    read errors < ${CMON_TERMINATE_ERROR_FILE}
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
#                 MAIN SCRIPT STARTS HERE ...
################################################################

# Create logfile if necessary
if [ ! -f ${CMON_WATCHDOG_LOG} ]; then
    touch ${CMON_WATCHDOG_LOG}
fi

# If application is not dead, then quit
app_pid=`pid_of_cmon`
if [ $? -eq 0 ]; then
    exit 0
fi

# If error file doesn't exists, then quit
if [ ! -f ${CMON_TERMINATE_ERROR_FILE} ]; then
    exit 0
fi

# If maximum number of terminate errors is reached, then fallback
terminate_errors=`get_terminate_errors`
if [ $terminate_errors -ge ${MAX_TERMINATE_ERRORS} ]; then
    date >> ${CMON_WATCHDOG_LOG}
    echo "About to start fallback" >> ${CMON_WATCHDOG_LOG}
    ${CMON_DIR}/${CMON_UTIL} fallback
    exit 0    
fi

# Time to reboot system
date >> ${CMON_WATCHDOG_LOG}
echo "About to reboot" >> ${CMON_WATCHDOG_LOG}
/sbin/reboot

exit 0
