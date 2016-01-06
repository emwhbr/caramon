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

#ifndef __IO_PIN_H__
#define __IO_PIN_H__

#include <stdint.h>
#include <string>

#include "gpio.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
// Return codes
#define IO_PIN_SUCCESS   0
#define IO_PIN_FAILURE  -1

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class io_pin {
 public:
  io_pin(string name,
	 uint8_t pin,
	 bool in,
	 bool active_low);

  virtual ~io_pin(void) = 0; // Pure virtual to make class abstract

  long initialize(void);

  long finalize(bool restore);

  string get_name(void);

  uint8_t get_pin(void);

 protected:
  string  m_name;
  uint8_t m_pin;
  bool    m_dir_in;
  bool    m_active_low;

  GPIO_FUNCTION m_old_pin_func;

 private:
};

#endif // __IO_PIN_H__
