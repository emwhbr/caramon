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

#ifndef __IO_PIN_IN_H__
#define __IO_PIN_IN_H__

#include "io_pin.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class io_pin_in : public io_pin {
 public:
  io_pin_in(string name,
	    uint8_t pin,
	    bool active_low);

  ~io_pin_in(void);

  long activated(bool &result);

 private:
};

#endif // __IO_PIN_IN_H__
