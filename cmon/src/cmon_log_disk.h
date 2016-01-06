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

#ifndef __CMON_LOG_DISK_H__
#define __CMON_LOG_DISK_H__

#include "cmon_log.h"
#include "cmon_file.h"

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
class cmon_log_disk : public cmon_log {
 public:
  cmon_log_disk(string name,
		bool verbose,
		string climate_data_file_name);
  ~cmon_log_disk(void);

  void initialize(void);  // Implements pure virtual functions from base class
  void finalize(void);    // Implements pure virtual functions from base class
  void log_climate_data(const CMON_CLIMATE_DATA *data); // Implements pure virtual functions from base class

 private:
  cmon_file *m_climate_data_file;
};

#endif // __CMON_LOG_DISK_H__
