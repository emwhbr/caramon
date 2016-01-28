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

#ifndef __CMON_POWER_SWITCH_RC_H__
#define __CMON_POWER_SWITCH_RC_H__

#include <stdint.h>

#include "cmon_power_switch.h"

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
class cmon_power_switch_rc : public cmon_power_switch {
 public:
  cmon_power_switch_rc(string name,
		       uint8_t system_code,
		       uint8_t unit_code,
		       bool verbose);
  ~cmon_power_switch_rc(void);

  void switch_on(void);  // Implements pure virtual functions from base class
  void switch_off(void); // Implements pure virtual functions from base class

 private:
  uint8_t m_system_code;
  uint8_t m_unit_code;

  void execute_tx433_cmd(const string cmd);
};

#endif // __CMON_POWER_SWITCH_RC_H__
