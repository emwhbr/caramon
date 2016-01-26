// ************************************************************************
// *                                                                      *
// * Copyright (C) 2015 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#ifndef __CMON_LOG_H__
#define __CMON_LOG_H__

#include <time.h>
#include <string>

#include "cmon_climate_data.h"
#include "cmon_controller_data.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_log {
 public:
  cmon_log(string name,
	   bool verbose);
  virtual ~cmon_log(void);
  
  // Pure virtual functions
  virtual void initialize(void) = 0;
  virtual void finalize(void) = 0;
  virtual void log_data(const CMON_CLIMATE_DATA *climate_data,
			const CMON_CONTROLLER_DATA *controller_data) = 0;  
 protected:
  string m_name;
  bool   m_verbose;

  string get_iso8601_date_time_str(const struct tm *tstruct);
};

#endif // __CMON_LOG_H__
