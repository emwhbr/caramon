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

#ifndef __CMON_EXT_SENSOR_H__
#define __CMON_EXT_SENSOR_H__

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of types
/////////////////////////////////////////////////////////////////////////////
// Available sensors
typedef enum {
  CMON_EXT_SENSOR_1,
  CMON_EXT_SENSOR_2,
  CMON_EXT_MAX_SENSORS
} CMON_EXT_SENSOR;


/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////

extern void cmon_ext_sensor_initialize(CMON_EXT_SENSOR sensor);

extern void cmon_ext_sensor_finalize(CMON_EXT_SENSOR sensor);

extern float cmon_ext_sensor_get_temperature(CMON_EXT_SENSOR sensor);

#endif // __CMON_EXT_SENSOR_H__
