// ************************************************************************
// *                                                                      *
// * Copyright (C) 2017 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include "cmon_wdt.h"
#include "io_pin_out.h"
#include "delay.h"
#include "cmon_exception.h"

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
// Available WDT pins
typedef enum {
  CMON_WDT_PIN_WDI,
  CMON_WDT_PIN_WDE,
  CMON_WDT_MAX_PINS
} CMON_WDT_PIN;

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
static io_pin_out m_wdt_pin[CMON_WDT_MAX_PINS] = {
  io_pin_out("WDI", PIN_WDI, false), // Active high (false)
  io_pin_out("WDE", PIN_WDE, false)  // Active high (false)
};

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

static void cmon_wdt_pin(CMON_WDT_PIN pin,
			 bool activate)
{
  long rc;

  if (activate) {
    rc =  m_wdt_pin[pin].activate();
  }
  else {
    rc = m_wdt_pin[pin].deactivate();
  }

  if (rc != IO_PIN_SUCCESS) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
              "Action %s for %s pin(%u) failed",
              (activate ? "ACTIVATE" : "DEACTIVATE"),
              m_wdt_pin[pin].get_name().c_str(),
              m_wdt_pin[pin].get_pin());
    }
}

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void cmon_wdt_initialize(void)
{
  long rc;

  // Initialize all WDT pins
  for (unsigned i=0; i < CMON_WDT_MAX_PINS; i++) {
    rc = m_wdt_pin[i].initialize();
    if (rc != IO_PIN_SUCCESS) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
		"Initialize %s pin(%u) failed",
		m_wdt_pin[i].get_name().c_str(), m_wdt_pin[i].get_pin());
    }
  }

  // Disable WDT.
  // Already pulled down by resistor at this stage.
  // But better safe than sorry.
  cmon_wdt_pin(CMON_WDT_PIN_WDE, false); // Deactivate (false)

  // Make sure function 'wdt_keep_alive' starts from low.
  // Already pulled down by resistor at this stage.
  // But better safe than sorry.
  cmon_wdt_pin(CMON_WDT_PIN_WDI, false); // Deactivate (false)
}

/////////////////////////////////////////////////////////////////////////////

void cmon_wdt_finalize(void)
{
  long rc;

  // Disable WDT
  cmon_wdt_disable();

  // Finalize all WDT pins.
  for (unsigned i=0; i < CMON_WDT_MAX_PINS; i++) {
    rc = m_wdt_pin[i].finalize(true); // Restore pin (true)
    if (rc != IO_PIN_SUCCESS) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
		"Finalize %s pin(%u) failed",
		m_wdt_pin[i].get_name().c_str(),
		m_wdt_pin[i].get_pin());
    }
  }  
}

/////////////////////////////////////////////////////////////////////////////

void cmon_wdt_enable(void)
{
  cmon_wdt_pin(CMON_WDT_PIN_WDE, true); // Activate (true)
}

/////////////////////////////////////////////////////////////////////////////

void cmon_wdt_disable(void)
{
  cmon_wdt_pin(CMON_WDT_PIN_WDE, false); // Deactivate (false)
}

/////////////////////////////////////////////////////////////////////////////

void cmon_wdt_keep_alive(void)
{
  // A minimal pulse shall be enough.
  // The input stage of the WDT circuit reacts on a falling edge.

  // High
  cmon_wdt_pin(CMON_WDT_PIN_WDI, true); // Activate (false)

  if ( delay(WDI_HIGH_US / 1000000.0) != DELAY_SUCCESS ) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIME_ERROR,
	      "Delay operation failed for WDT", NULL);
  }

  // Low
  cmon_wdt_pin(CMON_WDT_PIN_WDI, false); // Deactivate (false)
}
