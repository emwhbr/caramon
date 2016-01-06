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

#include "cmon_log.h"
#include "cmon_io.h"
#include "cmon_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_log::cmon_log(string name,
		   bool verbose)
{
  if (verbose) {
    cmon_io_put("%s : cmon_log::cmon_log\n", name.c_str());
  }

  m_name = name;
  m_verbose = verbose;
}

////////////////////////////////////////////////////////////////

cmon_log::~cmon_log(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_log::~cmon_log\n", m_name.c_str());
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

string cmon_log::get_iso8601_date_time_str(const struct tm *tstruct)
{
  char date_time_str[40];

  // Current date/time, format is in ISO-8601 format:
  // YYYY-MM-DDThh:mm:ss<offset from UTC>
  if (strftime(date_time_str,
	       sizeof(date_time_str),
	       "%Y-%m-%dT%X%z",
	       tstruct) == 0) {

    THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIME_ERROR,
	      "%s : strftime failed", m_name.c_str());
  }

  return date_time_str;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

