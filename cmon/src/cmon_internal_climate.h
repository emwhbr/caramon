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

#ifndef __CMON_INTERNAL_CLIMATE_H__
#define __CMON_INTERNAL_CLIMATE_H__

#include "hdc1008_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_internal_climate {
public:
  cmon_internal_climate(void);
  ~cmon_internal_climate(void);

  void initialize(void);
  void finalize(void);

  float get_temperature(void);
  float get_humidity(void);

 private:
  // HDC1008 temperature/humidity sensor object pointer
  hdc1008_io *m_hdc1008_io_ptr;

  void handle_hdc1008_exception(long rc);
};

#endif // __CMON_INTERNAL_CLIMATE_H__
