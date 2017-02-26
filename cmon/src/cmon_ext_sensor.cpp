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

#include "cmon_ext_sensor.h"
#include "ds18b20_io.h"
#include "cmon_utility.h"
#include "cmon_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// DS18B20 temperature sensor
#define DS18B20_SENSOR_1_SERIAL_NUMBER  "28-0516949b08ff"  // marked as T1 (new 2017)
#define DS18B20_SENSOR_2_SERIAL_NUMBER  "28-0215535bfbff"  // marked as T2 (old 2016)

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
// DS18B20 temperature sensor object
static ds18b20_io m_ds18b20_sensor[CMON_EXT_MAX_SENSORS] = {
  ds18b20_io(DS18B20_SENSOR_1_SERIAL_NUMBER),
  ds18b20_io(DS18B20_SENSOR_2_SERIAL_NUMBER)
};

// Protection for simultaneous access
static pthread_mutex_t m_ds18b20_mutex = PTHREAD_MUTEX_INITIALIZER;

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

static void handle_ds18b20_exception(long rc,
				     CMON_EXT_SENSOR sensor)
{
  switch (rc) {
  case DS18B20_IO_SUCCESS:
    return; // No exception on success
  case DS18B20_IO_FILE_OPERATION_FAILED:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20(T%d) file operation failed", sensor + 1);
  case DS18B20_IO_SENSOR_NOT_FOUND:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20(T%d) not found", sensor + 1);
  case DS18B20_IO_BAD_CRC:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20(T%d) bad crc", sensor + 1);
  case DS18B20_IO_INVALID_TEMPERATURE:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20(T%d) invalid temperature", sensor + 1);
  default:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20(T%d) unknown error(rc=%d)", rc, sensor + 1);
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_ext_sensor_initialize(CMON_EXT_SENSOR sensor)
{
  long rc;

  // Initialize DS18B20 temperature sensor
  rc = m_ds18b20_sensor[sensor].initialize();
  handle_ds18b20_exception(rc, sensor);
}

////////////////////////////////////////////////////////////////

void cmon_ext_sensor_finalize(CMON_EXT_SENSOR sensor)
{
  long rc;

  // Finalize DS18B20 temperature sensor
  rc = m_ds18b20_sensor[sensor].finalize();
  handle_ds18b20_exception(rc, sensor);
}

////////////////////////////////////////////////////////////////

float cmon_ext_sensor_get_temperature(CMON_EXT_SENSOR sensor)
{
  long rc;
  float value;

  cmon_mutex_lock(&m_ds18b20_mutex); // Lock-down sensor
  try {
    // Read temperature from DS18B20 sensor
    rc = m_ds18b20_sensor[sensor].read_temperature(value);
    handle_ds18b20_exception(rc, sensor);
  }
  catch (...) {
    cmon_mutex_unlock(&m_ds18b20_mutex);
    throw;
  }
  cmon_mutex_unlock(&m_ds18b20_mutex); // Lock-up sensor
  return value;
}
