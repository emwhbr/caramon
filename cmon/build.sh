#!/bin/bash
# /************************************************************************
#  *                                                                      *
#  * Copyright (C) 2015 Bonden i Nol (hakanbrolin@hotmail.com)            *
#  *                                                                      *
#  * This program is free software; you can redistribute it and/or modify *
#  * it under the terms of the GNU General Public License as published by *
#  * the Free Software Foundation; either version 2 of the License, or    *
#  * (at your option) any later version.                                  *
#  *                                                                      *
#  ************************************************************************/

################################################################
function get_parallel_args()
################################################################
{
    # Check number of CPU's in this machine
    nr_cpus=`cat /proc/cpuinfo | grep processor | wc -l`
    
    # Add one to get number of parallel jobs
    ((nr_jobs=nr_cpus + 1))
    
    echo "-j${nr_jobs}"
    return 0
}

################################################################
function print_usage_and_die()
################################################################
{
    echo "Usage: $0 {release|debug|clean}"
    echo ""
    echo "release   Build release, no debug support"
    echo "debug     Build with debug support"
    echo "clean     Build clean"
    exit 1  
}

####################################################################
##################             MAIN               ##################  
####################################################################

### Number of parallel jobs on this machine
PARALLEL_ARGS=`get_parallel_args`

### Check arguments
if [ "$#" -gt 1 ]; then
    print_usage_and_die
    exit 1
fi

MAKEFILE=./Makefile

### Build
case "$1" in
    release)
        echo "==[MAKE RELEASE]==="
        make -f ${MAKEFILE} JOBS=${PARALLEL_ARGS} BUILD_TYPE=RELEASE all
        ;;

    debug)
        echo "==[MAKE DEBUG]==="
        make -f ${MAKEFILE} JOBS=${PARALLEL_ARGS} BUILD_TYPE=DEBUG all
        ;;

    clean)
        echo "==[CLEANUP]==="
        make -f ${MAKEFILE} clean
        ;;

    *)
        print_usage_and_die
        ;;
esac
