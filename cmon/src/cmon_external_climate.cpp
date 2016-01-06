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

#include "cmon_external_climate.h"
#include "cmon_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
// DS18B20 temperature sensor
#define DS18B20_SENSOR_SERIAL_NUMBER  "28-0215535bfbff"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_external_climate::cmon_external_climate(void)
{
  // Create DS18B20 temperature sensor object
  m_ds18b20_io_ptr = new ds18b20_io(DS18B20_SENSOR_SERIAL_NUMBER);
}

////////////////////////////////////////////////////////////////

cmon_external_climate::~cmon_external_climate(void)
{
  // Destroy DS18B20 temperature sensor object
  delete m_ds18b20_io_ptr;
}

////////////////////////////////////////////////////////////////

void cmon_external_climate::initialize(void)
{
  long rc;

  // Initialize DS18B20 temperature sensor
  rc = m_ds18b20_io_ptr->initialize();
  this->handle_ds18b20_exception(rc);
}

////////////////////////////////////////////////////////////////

void cmon_external_climate::finalize(void)
{
  long rc;

  // Finalize DS18B20 temperature sensor
  rc = m_ds18b20_io_ptr->finalize();
  this->handle_ds18b20_exception(rc);
}

////////////////////////////////////////////////////////////////

float cmon_external_climate::get_temperature(void)
{
  long rc;
  float value;

  // Read temperature from DS18B20 sensor
  rc = m_ds18b20_io_ptr->read_temperature(value);
  this->handle_ds18b20_exception(rc);
  return value;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_external_climate::handle_ds18b20_exception(long rc)
{
  switch (rc) {
  case DS18B20_IO_SUCCESS:
    return; // No exception on success
  case DS18B20_IO_FILE_OPERATION_FAILED:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20 file operation failed", NULL);
  case DS18B20_IO_SENSOR_NOT_FOUND:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20 sensor not found", NULL);
  case DS18B20_IO_BAD_CRC:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20 bad crc", NULL);
  case DS18B20_IO_INVALID_TEMPERATURE:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20 invalid temperature", NULL);
  default:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "DS18B20 unknown error(rc=%d)", rc);
  }
}
