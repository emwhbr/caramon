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

#include "cmon_led.h"
#include "io_pin_out.h"
#include "cmon_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define PIN_LED_ALIVE    GPIO_P1_11
#define PIN_LED_SYSFAIL  GPIO_P1_12

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
static io_pin_out m_led_pin[CMON_LED_MAX_LEDS] = {
  io_pin_out("LED_ALIVE",   PIN_LED_ALIVE,   false), // Active high (false)
  io_pin_out("LED_SYSFAIL", PIN_LED_SYSFAIL, false)  // Active high (false)
};

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void cmon_led_initialize(void)
{
  long rc;

  // Initialize all LED pins
  for (unsigned i=0; i < CMON_LED_MAX_LEDS; i++) {
    rc = m_led_pin[i].initialize();
    if (rc != IO_PIN_SUCCESS) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
		"Initialize %s pin(%u) failed",
		m_led_pin[i].get_name().c_str(), m_led_pin[i].get_pin());
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void cmon_led_finalize(void)
{
  long rc;
  bool restore_pin;

  // Finalize all LED pins.
  // SYSFAIL shall not be restored if activated.
  for (unsigned i=0; i < CMON_LED_MAX_LEDS; i++) {
    if ( (i == CMON_LED_SYSFAIL) &&
	 (m_led_pin[i].activated()) ) {
      restore_pin = false;
    }
    else {
      restore_pin = true;
    }
    rc = m_led_pin[i].finalize(restore_pin);
    if (rc != IO_PIN_SUCCESS) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
		"Finalize %s pin(%u) failed",
		m_led_pin[i].get_name().c_str(),
		m_led_pin[i].get_pin());
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void cmon_led(CMON_LED led,
	      bool activate)
{
  long rc;

  if (activate) {
    rc =  m_led_pin[led].activate();
  }
  else {
    rc = m_led_pin[led].deactivate();
  }

  if (rc != IO_PIN_SUCCESS) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
	      "Action %s for %s pin(%u) failed",
	      (activate ? "ACTIVATE" : "DEACTIVATE"),
	      m_led_pin[led].get_name().c_str(),
	      m_led_pin[led].get_pin());
    }
}
