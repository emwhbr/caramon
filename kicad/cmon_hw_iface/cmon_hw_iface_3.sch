EESchema Schematic File Version 2
LIBS:linear-modified
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:db9-female
LIBS:sharp-relay
LIBS:74ac04
LIBS:maxim
LIBS:GrayCatLabs
LIBS:cmon_hw_iface-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 4
Title ""
Date "20 jan 2017"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Notes 3750 2400 0    60   ~ 0
1-WIRE
Text GLabel 4250 2650 0    60   Input ~ 0
P1-22
Text Notes 2150 3300 0    60   ~ 0
Raspberry Pi\n\nP1 GPIO (3.3V)
Wire Wire Line
	4250 2650 7100 2650
$Comp
L DS18B20 U5
U 1 1 58691799
P 5900 1750
F 0 "U5" H 5750 2000 50  0000 C CNN
F 1 "DS18B20" H 5900 1500 50  0000 C CNN
F 2 "" H 5750 2000 50  0000 C CNN
F 3 "" H 5750 2000 50  0000 C CNN
	1    5900 1750
	0    -1   -1   0   
$EndComp
$Comp
L DS18B20 U6
U 1 1 5869179F
P 7100 1750
F 0 "U6" H 6950 2000 50  0000 C CNN
F 1 "DS18B20" H 7100 1500 50  0000 C CNN
F 2 "" H 6950 2000 50  0000 C CNN
F 3 "" H 6950 2000 50  0000 C CNN
	1    7100 1750
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR?
U 1 1 586917A5
P 6000 2250
F 0 "#PWR?" H 6000 2250 30  0001 C CNN
F 1 "GND" H 6000 2180 30  0001 C CNN
F 2 "" H 6000 2250 60  0000 C CNN
F 3 "" H 6000 2250 60  0000 C CNN
	1    6000 2250
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR?
U 1 1 586917AB
P 4900 1000
F 0 "#PWR?" H 4900 960 30  0001 C CNN
F 1 "+3.3V" H 4900 1110 30  0000 C CNN
F 2 "" H 4900 1000 60  0000 C CNN
F 3 "" H 4900 1000 60  0000 C CNN
	1    4900 1000
	1    0    0    -1  
$EndComp
$Comp
L R R20
U 1 1 586917B1
P 4900 1800
F 0 "R20" V 4980 1800 40  0000 C CNN
F 1 "4.7k" V 4907 1801 40  0000 C CNN
F 2 "~" V 4830 1800 30  0000 C CNN
F 3 "~" H 4900 1800 30  0000 C CNN
	1    4900 1800
	-1   0    0    1   
$EndComp
Wire Wire Line
	4900 1000 4900 1550
Wire Wire Line
	5800 2050 5800 2250
Wire Wire Line
	5800 2250 5300 2250
Wire Wire Line
	5300 2250 5300 1250
Wire Wire Line
	4900 1250 6500 1250
Connection ~ 4900 1250
Wire Wire Line
	7000 2050 7000 2250
Wire Wire Line
	7000 2250 6500 2250
Wire Wire Line
	6500 2250 6500 1250
Connection ~ 5300 1250
Wire Wire Line
	6000 2250 6000 2050
$Comp
L GND #PWR?
U 1 1 586917E0
P 7200 2250
F 0 "#PWR?" H 7200 2250 30  0001 C CNN
F 1 "GND" H 7200 2180 30  0001 C CNN
F 2 "" H 7200 2250 60  0000 C CNN
F 3 "" H 7200 2250 60  0000 C CNN
	1    7200 2250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 2250 7200 2050
Wire Wire Line
	7100 2650 7100 2050
Wire Wire Line
	4900 2050 4900 2650
Connection ~ 4900 2650
Wire Wire Line
	5900 2050 5900 2650
Connection ~ 5900 2650
$Comp
L HDC1008 IC1
U 1 1 58691AA1
P 5800 4400
F 0 "IC1" H 5600 4750 60  0000 C CNN
F 1 "HDC1008" H 5900 4050 60  0000 C CNN
F 2 "" H 6000 4250 60  0000 C CNN
F 3 "" H 6000 4250 60  0000 C CNN
	1    5800 4400
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 58691AAE
P 6600 5100
F 0 "#PWR?" H 6600 5100 30  0001 C CNN
F 1 "GND" H 6600 5030 30  0001 C CNN
F 2 "" H 6600 5100 60  0000 C CNN
F 3 "" H 6600 5100 60  0000 C CNN
	1    6600 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 4350 6600 5100
$Comp
L +3.3V #PWR?
U 1 1 58691AB5
P 6600 3650
F 0 "#PWR?" H 6600 3610 30  0001 C CNN
F 1 "+3.3V" H 6600 3760 30  0000 C CNN
F 2 "" H 6600 3650 60  0000 C CNN
F 3 "" H 6600 3650 60  0000 C CNN
	1    6600 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 3650 6600 4200
Wire Wire Line
	6300 4600 6600 4600
Wire Wire Line
	6300 4350 6600 4350
Connection ~ 6600 4600
Wire Wire Line
	6300 4450 6600 4450
Connection ~ 6600 4450
Wire Wire Line
	6600 4200 6300 4200
Text Notes 7000 4450 0    60   ~ 0
ADR0=ADR1=0\n\nADDRESS 0x40
Text Notes 5350 3850 0    60   ~ 0
TEMP/HUM SENSOR
Text GLabel 4250 4100 0    60   Input ~ 0
P1-05
Wire Wire Line
	5300 4200 4900 4200
Wire Wire Line
	4900 4100 4900 4200
Wire Wire Line
	4250 4100 4900 4100
Text GLabel 4250 4400 0    60   Input ~ 0
P1-03
Wire Wire Line
	5300 4300 4900 4300
Wire Wire Line
	4900 4300 4900 4400
$Comp
L DS3231 IC2
U 1 1 5869202F
P 5800 6200
F 0 "IC2" H 5700 6675 50  0000 R CNN
F 1 "DS3231" H 5700 6600 50  0000 R CNN
F 2 "Housings_SOIC:SOIC-16_7.5x10.3mm_Pitch1.27mm" H 5850 5800 50  0001 L CNN
F 3 "" H 6070 6450 50  0001 C CNN
	1    5800 6200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5300 6100 4700 6100
Wire Wire Line
	4700 6100 4700 4100
Connection ~ 4700 4100
Wire Wire Line
	5300 6200 4550 6200
Wire Wire Line
	4550 6200 4550 4400
Connection ~ 4550 4400
Wire Wire Line
	4900 4400 4250 4400
Text Notes 5350 5500 0    60   ~ 0
RTC
Text Notes 3750 3850 0    60   ~ 0
I2C
Text Notes 5700 1000 0    60   ~ 0
TEMP SENSORS
Text Notes 7000 6050 0    60   ~ 0
ADDRESS 0x68
$EndSCHEMATC
