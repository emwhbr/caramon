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

#include "io_pin_out.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

io_pin_out::io_pin_out(string name,
		       uint8_t pin,
		       bool active_low) : io_pin(name,
						 pin,
						 false, // Dir = OUT (false)
						 active_low)
{
  m_activated = false;
}

////////////////////////////////////////////////////////////////

io_pin_out::~io_pin_out()
{
}

////////////////////////////////////////////////////////////////

long io_pin_out::activate(void)
{
  long rc;
  uint8_t value;

  if (m_active_low) {
    value = 0;
  }
  else {
    value = 1;
  }

  rc = gpio_write(m_pin, value);
  if (rc != GPIO_SUCCESS) {
    return IO_PIN_FAILURE;
  }

  m_activated = true;

  return IO_PIN_SUCCESS;
}

////////////////////////////////////////////////////////////////

long io_pin_out::deactivate(void)
{
  long rc;
  uint8_t value;

  if (m_active_low) {
    value = 1;
  }
  else {
    value = 0;
  }

  rc = gpio_write(m_pin, value);
  if (rc != GPIO_SUCCESS) {
    return IO_PIN_FAILURE;
  }

  m_activated = false;

  return IO_PIN_SUCCESS;
}

////////////////////////////////////////////////////////////////

bool io_pin_out::activated(void)
{
  return m_activated;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
