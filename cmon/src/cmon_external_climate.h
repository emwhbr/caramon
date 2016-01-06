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

#ifndef __CMON_EXTERNAL_CLIMATE_H__
#define __CMON_EXTERNAL_CLIMATE_H__

#include "ds18b20_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_external_climate {
public:
  cmon_external_climate(void);
  ~cmon_external_climate(void);

  void initialize(void);
  void finalize(void);

  float get_temperature(void);

 private:
  // DS18B20 temperature sensor object pointer
  ds18b20_io *m_ds18b20_io_ptr;

  void handle_ds18b20_exception(long rc);
};

#endif // __CMON_EXTERNAL_CLIMATE_H__
