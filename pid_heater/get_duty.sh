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

#### Usage:  ./get_duty.sh <pidX.data>

#### Cycle info, file pidX.data

#Cycles=84, Total time[min]=839.999920
#PID=34.550081, P=-2.566863, I=37.116943, D=0.000000
#DUTY=34.550081, HI=207.300483, LO=392.699517, temp=12.015686
#Heater ON
#ON time[s]=207.289147
#Heater OFF
#OFF time[s]=392.699351

cat $1 | awk -F "=" '/DUTY/ {print $2}' | awk -F "," '{print $1}'
