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

#ifndef __CMON_POWER_SWITCH_SSR_H__
#define __CMON_POWER_SWITCH_SSR_H__

#include <stdint.h>

#include "cmon_power_switch.h"
#include "io_pin_out.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_power_switch_ssr : public cmon_power_switch {
 public:
  cmon_power_switch_ssr(string name,
			uint8_t pin,
			bool verbose);
  ~cmon_power_switch_ssr(void);

  void initialize(void);
  void finalize(void);

  void switch_on(void);  // Implements pure virtual functions from base class
  void switch_off(void); // Implements pure virtual functions from base class

 private:
  io_pin_out *m_ssr_pin;
};

#endif // __CMON_POWER_SWITCH_SSR_H__
