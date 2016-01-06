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

#include "io_pin_in.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

io_pin_in::io_pin_in(string name,
		     uint8_t pin,
		     bool active_low) : io_pin(name,
					       pin,
					       true, // Dir = IN (true)
					       active_low)
{
}

////////////////////////////////////////////////////////////////

io_pin_in::~io_pin_in()
{
}

////////////////////////////////////////////////////////////////

long io_pin_in::activated(bool &result)
{
  long rc;
  uint8_t value;

  rc = gpio_read(m_pin, value);
  if (rc != GPIO_SUCCESS) {
    return IO_PIN_FAILURE;
  }

  if (m_active_low) {
    result = (value == 0);
  }
  else {
    result = (value == 1);
  }

  return IO_PIN_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
