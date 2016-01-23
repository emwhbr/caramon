// ************************************************************************
// *                                                                      *
// * Copyright (C) 2016 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include <stdio.h>
#include <stdlib.h>

#include "hdc1008_io.h"
#include "pid_ctrl.h"
#include "delay.h"
#include "timer.h"
#include "shell_cmd.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Wall outlet (240V) control
#define TX433_CMD          "/caramon/tx433"
#define TX433_CMD_EXIT_OK  0

#define TX433_SYSTEM_CODE  "4"
#define TX433_UNIT_CODE    "3"

#define TX433_ON   "1"
#define TX433_OFF  "0"

#define TX433_MIN_CTRL_TIME  5.0  // [s] Minimum time for activation/deactivation

// HDC1008 temperature/humidity sensor
#define HDC1008_I2C_ADDR  0x40          // ADR0 = ADR1 = GND
#define HDC1008_I2C_DEV   "/dev/i2c-1"  // Raspberry Pi 2 (Model B, GPIO P1)

// PID tuning
#define PID_VER "PID-8"
// KP = PID-7:163.64 // PID-6:211.76 // PID-5:163.64 // PID-4:360.0 // PID-3:270.0 // PID-2:90.0  // PID-1:45.00
// KI = PID-7: 42.10 // PID-6:121.01 // PID-5: 56.10 // PID-4:  0.0 // PID-3:  0.0 // PID-2: 0.0  // PID-1: 0.05
// KD = PID-7:  0.00 // PID-6: 92.65 // PID-5:  0.00 // PID-4:  0.0 // PID-3:  0.0 // PID-2: 0.0  // PID-1: 0.00
#define KP  163.64
#define KI   28.05  // PID-5 -50%
#define KD    0.00

/////////////////////////////////////////////////////////////////////////////
//               Definition of types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////
static shell_cmd m_sc;

////////////////////////////////////////////////////////////////

static void execute_tx433_cmd(const string cmd)
{
  int exit_status;

  // Execute command
  if (m_sc.execute(cmd, exit_status) != SHELL_CMD_SUCCESS) {
    printf("*** Failed to execute command : %s\n", cmd.c_str());
    throw 1;
  }

  // Check command exit status
  if (exit_status != TX433_CMD_EXIT_OK) {
    printf("*** Command failed : %s, exit status : %d",
	   cmd.c_str(), exit_status);
    throw 1;
  }
}

////////////////////////////////////////////////////////////////

static void heater_on(void)
{
  const string heater_on_cmd =
    string(TX433_CMD) + " " +
    string(TX433_SYSTEM_CODE) + " " +
    string(TX433_UNIT_CODE) + " " +
    string(TX433_ON);

  printf("Heater ON\n");

  execute_tx433_cmd(heater_on_cmd);
}

////////////////////////////////////////////////////////////////

static void heater_off(void)
{
  const string heater_off_cmd =
    string(TX433_CMD) + " " +
    string(TX433_SYSTEM_CODE) + " " +
    string(TX433_UNIT_CODE) + " " +
    string(TX433_OFF);

  printf("Heater OFF\n");

  execute_tx433_cmd(heater_off_cmd);
}

////////////////////////////////////////////////////////////////

void handle_hdc1008_exception(long rc)
{
  switch (rc) {
  case HDC1008_IO_SUCCESS:
    return; // No exception on success
  case HDC1008_IO_FILE_OPERATION_FAILED:
    printf("*** HDC1008 file operation failed\n");
    throw 1;
  case HDC1008_IO_UNEXPECTED_STATE:
    printf("*** HDC1008 unexpected state\n");
    throw 1;
  case HDC1008_IO_I2C_OPERATION_FAILED:
    printf("*** HDC1008 i2c operation failed\n");
    throw 1;
  default:
    printf("*** HDC1008 unknown error(rc=%ld\n)", rc);
    throw 1;
  }
}

////////////////////////////////////////////////////////////////

static void pid_loop(int ptime, int command_temp)
{
  long rc;

  hdc1008_io hdc1008_sensor(HDC1008_I2C_ADDR,
			    HDC1008_I2C_DEV);
  float temperature;

  pid_ctrl pid_heater(command_temp,
		      KP,
		      KI,
		      KD,
		      PID_CTRL_DIRECT);
  double duty;
  double t_hi;
  double t_lo;

  const clockid_t the_clock = get_clock_id();
  struct timespec t1;
  struct timespec t2;

  timer total_timer;
  timer pulse_timer;
  
  int cycles = 0;

  // Initialize HDC1008 temperature/humidity sensor
  rc = hdc1008_sensor.initialize();
  handle_hdc1008_exception(rc);

  // Initialize PID
  pid_heater.set_output_limits(0.0, 100.0);
  pid_heater.set_command_position((double)command_temp);

  // Get start time  
  if (clock_gettime(the_clock, &t1) == -1) {
    throw 1;
  }
  total_timer.reset();
  
  heater_off(); // Start with heater OFF

  // Do PID controller
  while (1) {
    printf("Cycles=%d, Total time[min]=%f\n",
	   cycles++, total_timer.get_elapsed_time() / 60.0);

    // Read temperature from HDC1008 sensor
    rc = hdc1008_sensor.read_temperature(temperature);
    handle_hdc1008_exception(rc);

    // Update PID
    duty = pid_heater.update((double)temperature);

    // Apply new output
    t_hi = ptime * 60.0 * duty / 100.0;
    t_lo = ptime * 60.0 - t_hi;
    printf("DUTY=%f, HI=%f, LO=%f, temp=%f\n",
	   duty, t_hi, t_lo, temperature);

    // Turn heater ON
    pulse_timer.reset();
    if (t_hi > TX433_MIN_CTRL_TIME) {
      heater_on();
    }
    else {
      printf("Heater ON skipped\n");
    }
    if (get_new_time(&t1, t_hi, &t2) != DELAY_SUCCESS ) {
      printf("*** (ON) get_new_time failed\n");
      throw 1;
    }
    if (delay_until(&t2) != DELAY_SUCCESS) {
      printf("*** (ON) delay_until failed\n");
      throw 1;
    }
    printf("ON time[s]=%f\n", pulse_timer.get_elapsed_time());

    // Turn heater OFF
    pulse_timer.reset();
    if (t_lo > TX433_MIN_CTRL_TIME) {
      heater_off();
    }
    else {
      printf("Heater OFF skipped\n");
    }
    t1.tv_sec = t2.tv_sec;
    t1.tv_nsec = t2.tv_nsec;
    if (get_new_time(&t1, t_lo, &t2) != DELAY_SUCCESS ) {
      printf("*** (OFF) get_new_time failed\n");
      throw 1;
    }
    if (delay_until(&t2) != DELAY_SUCCESS) {
      printf("*** (OFF) delay_until failed\n");
      throw 1;
    }
    printf("OFF time[s]=%f\n", pulse_timer.get_elapsed_time());

    // Prepare next cycle
    t1.tv_sec = t2.tv_sec;
    t1.tv_nsec = t2.tv_nsec;
  }
}


////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  int ptime;
  int command_temp;

  printf("%s\n", PID_VER);

  // Basic check of argument
  if (argc != 3) {
    printf("Usage: pid_heater <period-time> <set-temperature>\n");
    exit(EXIT_FAILURE);
  }
  ptime = atoi(argv[1]);
  command_temp = atoi(argv[2]);

  // Execute PID loop
  try {
    pid_loop(ptime, command_temp);
  }
  catch (...) {
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
