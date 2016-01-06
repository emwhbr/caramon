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

#include "cmon_internal_climate.h"
#include "cmon_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
// HDC1008 temperature/humidity sensor
#define HDC1008_I2C_ADDR  0x40          // ADR0 = ADR1 = GND
#define HDC1008_I2C_DEV   "/dev/i2c-1"  // Raspberry Pi 2 (Model B, GPIO P1)

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_internal_climate::cmon_internal_climate(void)
{
  // Create HDC1008 temperature/humidity sensor object
  m_hdc1008_io_ptr = new hdc1008_io(HDC1008_I2C_ADDR,
				    HDC1008_I2C_DEV);
}

////////////////////////////////////////////////////////////////

cmon_internal_climate::~cmon_internal_climate(void)
{
  // Destroy HDC1008 temperature/humidity sensor object
  delete m_hdc1008_io_ptr;
}

////////////////////////////////////////////////////////////////

void cmon_internal_climate::initialize(void)
{
  long rc;

  // Initialize HDC1008 temperature/humidity sensor
  rc = m_hdc1008_io_ptr->initialize();
  this->handle_hdc1008_exception(rc);
}

////////////////////////////////////////////////////////////////

void cmon_internal_climate::finalize(void)
{
  long rc;

  // Finalize HDC1008 temperature/humidity sensor
  rc = m_hdc1008_io_ptr->finalize();
  this->handle_hdc1008_exception(rc);
}

////////////////////////////////////////////////////////////////

float cmon_internal_climate::get_temperature(void)
{
  long rc;
  float value;

  // Read temperature from HDC1008 sensor
  rc = m_hdc1008_io_ptr->read_temperature(value);
  this->handle_hdc1008_exception(rc);
  return value;
}

////////////////////////////////////////////////////////////////

float cmon_internal_climate::get_humidity(void)
{
  long rc;
  float value;

  // Read humidity from HDC1008 sensor
  rc = m_hdc1008_io_ptr->read_humidity(value);
  this->handle_hdc1008_exception(rc);
  return value;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_internal_climate::handle_hdc1008_exception(long rc)
{
  switch (rc) {
  case HDC1008_IO_SUCCESS:
    return; // No exception on success
  case HDC1008_IO_FILE_OPERATION_FAILED:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "HDC1008 file operation failed", NULL);
  case HDC1008_IO_UNEXPECTED_STATE:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "HDC1008 unexpected state", NULL);
  case HDC1008_IO_I2C_OPERATION_FAILED:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "HDC1008 i2c operation failed", NULL);
  default:
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_CLIMATE_SENSOR_ERROR,
	      "HDC1008 unknown error(rc=%d)", rc);
  }
}
