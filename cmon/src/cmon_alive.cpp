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

#include "cmon_alive.h"
#include "cmon_io.h"
#include "cmon_exception.h"
#include "cmon_utility.h"
#include "cmon_led.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define ALIVE_INTERVAL  0.5 // [s]

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_alive::
cmon_alive(string thread_name,
	   uint32_t cpu_affinity_mask,
	   int rt_priority,
	   bool verbose) : thread(thread_name,
				  cpu_affinity_mask,
				  rt_priority)
{
  if (verbose) {
    cmon_io_put("%s : cmon_alive::cmon_alive\n",
		this->get_name().c_str());
  }

  m_verbose = verbose;
  m_shutdown_requested = false;
}

////////////////////////////////////////////////////////////////

cmon_alive::~cmon_alive(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_alive::~cmon_alive\n",
		this->get_name().c_str());
  }
}

////////////////////////////////////////////////////////////////

void cmon_alive::shutdown(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_alive::shutdown\n",
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

long cmon_alive::setup(void)
{
  // Perform setup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_alive::setup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->initialize_alive();

    if (m_verbose) {
      cmon_io_put("%s : setup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_alive::setup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_alive::setup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_alive::cleanup(void)
{
  // Perform cleanup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_alive::cleanup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->finalize_alive();

    if (m_verbose) {
      cmon_io_put("%s : cleanup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_alive::cleanup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_alive::cleanup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_alive::execute(void *arg)
{
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_alive:execute, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    if (arg) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_BAD_ARGUMENT,
		"Argument not NULL", NULL);
    }

    this->handle_alive();

    if (m_verbose) {
      cmon_io_put("%s : execute done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Execute OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_alive::execute, exception:\n%s\n",
		this->get_name().c_str(),
		gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_alive::execute, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_alive::initialize_alive(void)
{
  cmon_led(CMON_LED_ALIVE, true);  // Turn status LED on
}

////////////////////////////////////////////////////////////////

void cmon_alive::finalize_alive(void)
{
  cmon_led(CMON_LED_ALIVE, false);  // Turn status LED off
}

////////////////////////////////////////////////////////////////

void cmon_alive::handle_alive(void)
{
  bool led_active = true;

  while (!is_stopped()) {
    cmon_led(CMON_LED_ALIVE, led_active);  // Toggle status LED
    led_active = !led_active;
    cmon_nanosleep(ALIVE_INTERVAL);        // Take it easy
  }
}
