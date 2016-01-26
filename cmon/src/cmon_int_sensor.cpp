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

#include "cmon_int_sensor.h"
#include "hdc1008_io.h"
#include "cmon_utility.h"
#include "cmon_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// HDC1008 temperature/humidity sensor
#define HDC1008_I2C_ADDR  0x40          // ADR0 = ADR1 = GND
#define HDC1008_I2C_DEV   "/dev/i2c-1"  // Raspberry Pi 2 (Model B, GPIO P1)

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
// HDC1008 temperature/humidity sensor object
static hdc1008_io m_hdc1008_sensor(HDC1008_I2C_ADDR,
				   HDC1008_I2C_DEV);

// Protection for simultaneous access
static pthread_mutex_t m_hdc1008_mutex = PTHREAD_MUTEX_INITIALIZER;

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

static void handle_hdc1008_exception(long rc)
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

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_int_sensor_initialize(void)
{
  long rc;

  // Initialize HDC1008 temperature/humidity sensor
  rc = m_hdc1008_sensor.initialize();
  handle_hdc1008_exception(rc);
}

////////////////////////////////////////////////////////////////

void cmon_int_sensor_finalize(void)
{
  long rc;

  // Finalize HDC1008 temperature/humidity sensor
  rc = m_hdc1008_sensor.finalize();
  handle_hdc1008_exception(rc);
}

////////////////////////////////////////////////////////////////

float cmon_int_sensor_get_temperature(void)
{
  long rc;
  float value;

  cmon_mutex_lock(&m_hdc1008_mutex); // Lock-down sensor
  try {
    // Read temperature from HDC1008 sensor
    rc = m_hdc1008_sensor.read_temperature(value);
    handle_hdc1008_exception(rc);
  }
  catch (...) {
    cmon_mutex_unlock(&m_hdc1008_mutex);
  }
  cmon_mutex_unlock(&m_hdc1008_mutex); // Lock-up sensor
  return value;
}

////////////////////////////////////////////////////////////////

float cmon_int_sensor_get_humidity(void)
{
  long rc;
  float value;

  cmon_mutex_lock(&m_hdc1008_mutex); // Lock-down sensor
  try {
    // Read humidity from HDC1008 sensor
    rc = m_hdc1008_sensor.read_humidity(value);
    handle_hdc1008_exception(rc);
  }
  catch (...) {
    cmon_mutex_unlock(&m_hdc1008_mutex);
  }
  cmon_mutex_unlock(&m_hdc1008_mutex); // Lock-up sensor
  return value;
}
