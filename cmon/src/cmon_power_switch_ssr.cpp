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

#include "cmon_power_switch_ssr.h"
#include "cmon_exception.h"
#include "cmon_io.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_power_switch_ssr::
cmon_power_switch_ssr(string name,
		     uint8_t pin,
		     bool verbose) : cmon_power_switch(name, verbose) 
{
  m_ssr_pin = new io_pin_out(name, pin, false); // Active high (false)
}

////////////////////////////////////////////////////////////////

cmon_power_switch_ssr::~cmon_power_switch_ssr(void)
{
  delete m_ssr_pin;
}

////////////////////////////////////////////////////////////////

void cmon_power_switch_ssr::initialize(void)
{
  long rc;

  rc = m_ssr_pin->initialize();
  if (rc != IO_PIN_SUCCESS) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
	      "Initialize %s pin(%u) failed",
	      m_ssr_pin->get_name().c_str(), m_ssr_pin->get_pin());
  }
}

////////////////////////////////////////////////////////////////

void cmon_power_switch_ssr::finalize(void)
{
  long rc;

  rc = m_ssr_pin->finalize(true);  // Restore pin (true)
  if (rc != IO_PIN_SUCCESS) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
	      "Finalize %s pin(%u) failed",
	      m_ssr_pin->get_name().c_str(), m_ssr_pin->get_pin());
  }
}

////////////////////////////////////////////////////////////////

void cmon_power_switch_ssr::switch_on(void)
{
  long rc;

  rc = m_ssr_pin->activate();
  if (rc != IO_PIN_SUCCESS) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
	      "Activate %s pin(%u) failed",
	      m_ssr_pin->get_name().c_str(), m_ssr_pin->get_pin());
  }
}

////////////////////////////////////////////////////////////////

void cmon_power_switch_ssr::switch_off(void)
{
  long rc;

  rc = m_ssr_pin->deactivate();
  if (rc != IO_PIN_SUCCESS) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_GPIO_ERROR,
	      "Deactivate %s pin(%u) failed",
	      m_ssr_pin->get_name().c_str(), m_ssr_pin->get_pin());
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
