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

#ifndef __CMON_POWER_SWITCH_H__
#define __CMON_POWER_SWITCH_H__

#include <string>

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
class cmon_power_switch {
 public:
  cmon_power_switch(string name,
		    bool verbose);
  virtual ~cmon_power_switch(void);

  virtual void switch_on(void) = 0;  // Pure virtual function
  virtual void switch_off(void) = 0; // Pure virtual function

 protected:
  string m_name;
  bool m_verbose;
  
 private:
};

#endif // __CMON_POWER_SWITCH_H__
