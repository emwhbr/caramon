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
#include "gpio.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define PIN_WDI  GPIO_P1_16  // BCM GPIO 23
                             // This pin keeps the WDT alive

#define PIN_WDE  GPIO_P1_18  // BCM GPIO 24
                             // This pin enables the WDT

#define WDI_HIGH_US  100     // Time[us] WDI pulse high
                             // All values below was long enough to create a
                             // falling edge on input stage of the WDT circuit.
                             // Set    Real (measured with oscilloscope)
                             //   0    75
                             //   1    75
                             // 100   250
                             // 300   380
                             // 500   580

/////////////////////////////////////////////////////////////////////////////
//               Definition of types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////
static gpio g_gpio;

////////////////////////////////////////////////////////////////

static void wdt_initialize(void)
{
  long rc;

  rc = g_gpio.initialize();
  if (rc != GPIO_SUCCESS) {
    printf("*** GPIO initialize failed, rc=%ld\n", rc);
    throw 1;
  }

  // WDE
  rc = g_gpio.set_function(PIN_WDE, GPIO_FUNC_OUT);
  if (rc != GPIO_SUCCESS) {
    printf("*** GPIO set_function failed for WDE, pin(%d), rc=%ld\n",
	   PIN_WDE, rc);
    throw 1;
  }
  rc = g_gpio.write(PIN_WDE, 0); // Disable WDT.
                                 // Already pulled down by resistor at this stage.
                                 // But better safe than sorry.
  if (rc != GPIO_SUCCESS) {
    printf("*** GPIO write (low) failed for WDE, pin(%d), rc=%ld\n",
	   PIN_WDE, rc);
    throw 1;
  }

  // WDI
  rc = g_gpio.set_function(PIN_WDI, GPIO_FUNC_OUT);
  if (rc != GPIO_SUCCESS) {
    printf("*** GPIO set_function failed for WDI, pin(%d), rc=%ld\n",
	   PIN_WDI, rc);
    throw 1;
  }
  rc = g_gpio.write(PIN_WDI, 0); // Make sure function 'wdt_keep_alive' starts from low.
                                 // Already pulled down by resistor at this stage.
                                 // But better safe than sorry.
  if (rc != GPIO_SUCCESS) {
    printf("*** GPIO write (low) failed for WDI, pin(%d), rc=%ld\n",
	   PIN_WDI, rc);
    throw 1;
  }
}

////////////////////////////////////////////////////////////////

static void wdt_keep_alive(void)
{
  long rc;

  // A minimal pulse shall be enough.
  // The input stage of the WDT circuit reacts on a falling edge.

  // High
  rc = g_gpio.write(PIN_WDI, 1);
  if (rc != GPIO_SUCCESS) {
    printf("*** GPIO write (high) failed for WDI, pin(%d), rc=%ld\n",
	   PIN_WDI, rc);
    throw 1;
  }
  if ( delay(WDI_HIGH_US / 1000000.0) != DELAY_SUCCESS ) {
    printf("*** delay failed\n");
    throw 1;
  }
  
  // Low
  rc = g_gpio.write(PIN_WDI, 0);
  if (rc != GPIO_SUCCESS) {
    printf("*** GPIO write (low) failed for WDI, pin(%d), rc=%ld\n",
	   PIN_WDI, rc);
    throw 1;
  }
}

////////////////////////////////////////////////////////////////

static void wdt_loop(int ptime_ms)
{
  long rc;

  const clockid_t the_clock = get_clock_id();
  struct timespec t1;
  struct timespec t2;

  int cycles = 0;

  wdt_initialize();                             // Initialize watchdog timer
  if (clock_gettime(the_clock, &t1) == -1) {    // Get start time 
    printf("*** clock_gettime failed\n");
    throw 1;
  }

  // Enable the watchdog timer
  rc = g_gpio.write(PIN_WDE, 1);
  if (rc != GPIO_SUCCESS) {
    printf("*** GPIO write (high) failed for WDE, pin(%d), rc=%ld\n",
	   PIN_WDE, rc);
    throw 1;
  }

  // "Kicking the dog" forever and ever
  while (1) {
    printf("WDT-cycle=%d\n", cycles++);
    wdt_keep_alive();

    // Take it easy until next cycle
    if (get_new_time(&t1, ptime_ms / 1000.0, &t2) != DELAY_SUCCESS ) {
      printf("*** get_new_time failed\n");
      throw 1;
    }
    if (delay_until(&t2) != DELAY_SUCCESS) {
      printf("*** delay_until failed\n");
      throw 1;
    }

    // Prepare next cycle
    t1.tv_sec  = t2.tv_sec;
    t1.tv_nsec = t2.tv_nsec;
  }
}


////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  int ptime_ms;

  // Basic check of argument
  if (argc != 2) {
    printf("Usage: wdt_test <period-time-ms>\n");
    exit(EXIT_FAILURE);
  }
  ptime_ms = atoi(argv[1]);

  // Execute watchdog loop
  try {
    wdt_loop(ptime_ms);
  }
  catch (...) {
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
