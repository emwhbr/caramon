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

#include "cmon_fallback.h"
#include "cmon.h"
#include "cmon_io.h"
#include "cmon_exception.h"
#include "cmon_utility.h"
#include "cmon_led.h"
#include "cmon_power_switch_ssr.h"
#include "timer.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define TEMP_PWM_PERIOD_TIME_SEC  60.0 // [s]
#define TEMP_PWM_DUTY             25.0 // [%]

#define FALLBACK_LED_FREQUENCY  2.0 // [Hz]

#define PIN_RADIATOR_CTRL  GPIO_P1_15  // Controls radiator via SSR K1

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_fallback::
cmon_fallback(string thread_name,
	      uint32_t cpu_affinity_mask,
	      int rt_priority,
	      bool verbose) : thread(thread_name,
				     cpu_affinity_mask,
				     rt_priority)
{
  if (verbose) {
    cmon_io_put("%s : cmon_fallback::cmon_fallback\n",
		this->get_name().c_str());
  }

  m_verbose = verbose;
  m_fallback_led_active = false;
  m_shutdown_requested = false;

  m_radiator_switch = new cmon_power_switch_ssr(string("RADIATOR-CTRL"),
						PIN_RADIATOR_CTRL,
						m_verbose);  
}

////////////////////////////////////////////////////////////////

cmon_fallback::~cmon_fallback(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_fallback::~cmon_fallback\n",
		this->get_name().c_str());
  }
  delete m_radiator_switch;
}

////////////////////////////////////////////////////////////////

void cmon_fallback::shutdown(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_fallback::shutdown\n",
		this->get_name().c_str());
  }

  // Initiate a controlled shutdown
  m_shutdown_requested = true;

  this->stop();
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long cmon_fallback::setup(void)
{
  // Perform setup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_fallback::setup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->initialize_fallback();

    if (m_verbose) {
      cmon_io_put("%s : setup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_fallback::setup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_fallback::setup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_fallback::cleanup(void)
{
  // Perform cleanup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_fallback::cleanup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->finalize_fallback();

    if (m_verbose) {
      cmon_io_put("%s : cleanup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_fallback::cleanup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_fallback::cleanup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_fallback::execute(void *arg)
{
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_fallback:execute, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    if (arg) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_BAD_ARGUMENT,
		"Argument not NULL", NULL);
    }

    this->handle_fallback();

    if (m_verbose) {
      cmon_io_put("%s : execute done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Execute OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_fallback::execute, exception:\n%s\n",
		this->get_name().c_str(),
		gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_fallback::execute, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_fallback::initialize_fallback(void)
{
  cmon_led(CMON_LED_SYSFAIL, true);  // Turn status LED on

  // Initialize power switch (Only necessary for SSR controlled power switch)
  cmon_power_switch_ssr *ssr_switch = dynamic_cast<cmon_power_switch_ssr*>(m_radiator_switch);
  if (ssr_switch) {
    ssr_switch->initialize();
  }
  this->radiator_off(); // Turn off radiator
}

////////////////////////////////////////////////////////////////

void cmon_fallback::finalize_fallback(void)
{
  cmon_led(CMON_LED_SYSFAIL, false);  // Turn status LED off

  this->radiator_off(); // Turn off radiator

  // Finalize power switch (Only necessary for SSR controlled power switch)
  cmon_power_switch_ssr *ssr_switch = dynamic_cast<cmon_power_switch_ssr*>(m_radiator_switch);
  if (ssr_switch) {
    ssr_switch->finalize();
  }
}

////////////////////////////////////////////////////////////////

void cmon_fallback::handle_fallback(void)
{
  const double radiator_on_sec = TEMP_PWM_PERIOD_TIME_SEC * TEMP_PWM_DUTY / 100.0;
  const double radiator_off_sec = TEMP_PWM_PERIOD_TIME_SEC - radiator_on_sec;

  // Radiator PWM loop
  while (!is_stopped()) {
    this->radiator_pulse(true, radiator_on_sec);   // Turn radiator ON
    this->radiator_pulse(false, radiator_off_sec); // Turn radiator OFF
  }
}

////////////////////////////////////////////////////////////////

void cmon_fallback::radiator_pulse(bool on,
				   double pulse_time_sec)
{
  double led_start_time;
  timer pulse_timer;

  pulse_timer.reset();

  if (on) {
    this->radiator_on();
  }
  else {
    this->radiator_off();
  }
  if (m_verbose) {
    cmon_io_put("%s : radiator %s\n",
		this->get_name().c_str(),
		(on ? "ON" : "OFF"));
  }

  led_start_time = pulse_timer.get_elapsed_time();

  while (pulse_timer.get_elapsed_time() < pulse_time_sec) {
    if (is_stopped()) {
      break;
    }
    cmon_nanosleep(0.05); // Take it easy

    // Toggle status LED
    if ( (pulse_timer.get_elapsed_time() - led_start_time) > 
	 ( (1.0 / FALLBACK_LED_FREQUENCY) / 2.0) ) {
      led_start_time = pulse_timer.get_elapsed_time();
      cmon_led(CMON_LED_SYSFAIL, m_fallback_led_active);
      m_fallback_led_active = !m_fallback_led_active;
    }
  }
}

////////////////////////////////////////////////////////////////

void cmon_fallback::radiator_on(void)
{
  m_radiator_switch->switch_on();
}

////////////////////////////////////////////////////////////////

void cmon_fallback::radiator_off(void)
{
  m_radiator_switch->switch_off();
}
