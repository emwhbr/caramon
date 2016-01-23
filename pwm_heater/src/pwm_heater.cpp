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

#include "delay.h"
#include "timer.h"
#include "shell_cmd.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define TX433_CMD          "/caramon/tx433"
#define TX433_CMD_EXIT_OK  0

#define TX433_SYSTEM_CODE  "4"
#define TX433_UNIT_CODE    "3"

#define TX433_ON   "1"
#define TX433_OFF  "0"

/////////////////////////////////////////////////////////////////////////////
//               Definition of types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////
shell_cmd m_sc;

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

void heater_on(void)
{
  const string heater_on_cmd =
    string(TX433_CMD) + " " +
    string(TX433_SYSTEM_CODE) + " " +
    string(TX433_UNIT_CODE) + " " +
    string(TX433_ON);

  execute_tx433_cmd(heater_on_cmd);
}

////////////////////////////////////////////////////////////////

void heater_off(void)
{
  const string heater_off_cmd =
    string(TX433_CMD) + " " +
    string(TX433_SYSTEM_CODE) + " " +
    string(TX433_UNIT_CODE) + " " +
    string(TX433_OFF);

  execute_tx433_cmd(heater_off_cmd);
}

////////////////////////////////////////////////////////////////

static void pwm_loop(int ptime, int duty)
{
  const double t_hi = ptime * 60.0 * (double)duty / 100.0;
  const double t_lo = ptime * 60.0 - t_hi;

  const clockid_t the_clock = get_clock_id();
  struct timespec t1;
  struct timespec t2;

  timer total_timer;
  timer pulse_timer;

  int cycles = 0;

  // Get start time  
  if (clock_gettime(the_clock, &t1) == -1) {
    throw 1;
  }

  total_timer.reset();

  //heater_off();

  // Do PWM
  while (1) {
    printf("Cycles=%d, Total time[min]=%f\n",
	   cycles++, total_timer.get_elapsed_time() / 60.0);

    // Create HI pulse
    pulse_timer.reset();
    heater_on();
    if (get_new_time(&t1, t_hi, &t2) != DELAY_SUCCESS ) {
      printf("*** (HI) get_new_time failed\n");
      throw 1;
    }
    if (delay_until(&t2) != DELAY_SUCCESS) {
      printf("*** (HI) delay_until failed\n");
      throw 1;
    }
    printf("HI[s]=%f\n", pulse_timer.get_elapsed_time());

    // Create LO pulse
    pulse_timer.reset();
    heater_off();
    t1.tv_sec = t2.tv_sec;
    t1.tv_nsec = t2.tv_nsec;
    if (get_new_time(&t1, t_lo, &t2) != DELAY_SUCCESS ) {
      printf("*** (LO) get_new_time failed\n");
      throw 1;
    }
    if (delay_until(&t2) != DELAY_SUCCESS) {
      printf("*** (LO) delay_until failed\n");
      throw 1;
    }
    printf("LO[s]=%f\n", pulse_timer.get_elapsed_time());

    // Prepare next cycle
    t1.tv_sec = t2.tv_sec;
    t1.tv_nsec = t2.tv_nsec;
  }
}


////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  int ptime;
  int duty;

  // Basic check of argument
  if (argc != 3) {
    printf("Usage: pwm_heater <period-time> <duty>\n");
    exit(EXIT_FAILURE);
  }
  ptime = atoi(argv[1]);
  duty = atoi(argv[2]);

  // Execute PWM loop
  try {
    pwm_loop(ptime, duty);
  }
  catch (...) {
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
