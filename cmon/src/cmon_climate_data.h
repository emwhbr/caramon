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

#ifndef __CMON_CLIMATE_DATA_H__
#define __CMON_CLIMATE_DATA_H__

#include <time.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
typedef struct {
  struct tm tstruct;
} CMON_CLIMATE_TIME;

typedef struct {
  bool  valid;
  float min;
  float max;
  float mean;
} CMON_CLIMATE_ITEM;

typedef struct {
  CMON_CLIMATE_TIME time;
  CMON_CLIMATE_ITEM internal_temperature;
  CMON_CLIMATE_ITEM internal_humidity;
  CMON_CLIMATE_ITEM external_temperature_1;
  CMON_CLIMATE_ITEM external_temperature_2;
} CMON_CLIMATE_DATA;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_climate_data {
 public:
  cmon_climate_data(void);
  virtual ~cmon_climate_data(void);
  
  CMON_CLIMATE_DATA m_climate_data;
};

#endif // __CMON_CLIMATE_DATA_H__
