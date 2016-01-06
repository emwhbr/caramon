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

#include "io_pin.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

io_pin::io_pin(string name,
	       uint8_t pin,
	       bool in,
	       bool active_low)
{
  m_name = name;
  m_pin = pin;
  m_dir_in = in;
  m_active_low = active_low;
}

////////////////////////////////////////////////////////////////

io_pin::~io_pin()
{
  // Mandatory virtual destructor, even if it's empty.
}

////////////////////////////////////////////////////////////////

long io_pin::initialize(void)
{
  long rc;
  GPIO_FUNCTION pin_func;

  // Get current GPIO function for pin
  rc = gpio_get_function(m_pin, m_old_pin_func);
  if (rc != GPIO_SUCCESS) {
    return IO_PIN_FAILURE;
  }

  // Set new pin function (IN or OUT)
  if (m_dir_in) {
    pin_func = GPIO_FUNC_INP;
  }
  else {
    pin_func = GPIO_FUNC_OUT;
  }
  rc = gpio_set_function(m_pin, pin_func);
  if (rc != GPIO_SUCCESS) {
    return IO_PIN_FAILURE;
  }

  return IO_PIN_SUCCESS;
}

////////////////////////////////////////////////////////////////

long io_pin::finalize(bool restore)
{
  long rc;

  if (restore) {
    rc = gpio_set_function(m_pin, m_old_pin_func);
    if (rc != GPIO_SUCCESS) {
      return IO_PIN_FAILURE;
    }
  }

  return IO_PIN_SUCCESS;
}

////////////////////////////////////////////////////////////////

string io_pin::get_name(void)
{
  return m_name;
}

////////////////////////////////////////////////////////////////

uint8_t io_pin::get_pin(void)
{
  return m_pin;
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
