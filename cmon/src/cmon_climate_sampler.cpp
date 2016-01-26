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

#include <string.h>

#include "cmon_climate_sampler.h"
#include "cmon_int_sensor.h"
#include "cmon_ext_sensor.h"
#include "cmon_io.h"
#include "cmon_exception.h"
#include "cmon_utility.h"
#include "timer.h"
#include "delay.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define CLIMATE_SAMPLE_INTERVAL   5.0  // [s]
#define CLIMATE_LOG_INTERVAL     (5 * 60.0)  // [s]
//#define CLIMATE_LOG_INTERVAL     (10.0)  // [s] JOE: For testing

#define MAX_CONSECUTIVE_INTERNAL_CLIMATE_SENSOR_ERRORS  8
#define MAX_CONSECUTIVE_EXTERNAL_CLIMATE_SENSOR_ERRORS  8

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_climate_sampler::
cmon_climate_sampler(string thread_name,
		     uint32_t cpu_affinity_mask,
		     int rt_priority,
		     cmon_climate_data_queue *climate_data_queue,
		     bool verbose) : thread(thread_name,
					    cpu_affinity_mask,
					    rt_priority)
{
  if (verbose) {
    cmon_io_put("%s : cmon_climate_sampler::cmon_climate_sampler\n",
		this->get_name().c_str());
  }

  m_verbose = verbose;
  m_shutdown_requested = false;

  m_climate_data_queue = climate_data_queue;

  m_internal_climate_sensor_error_cnt = 0;
  m_internal_climate_sensor_permanent_fault = false;
  m_internal_temperature_stats.reset();
  m_internal_humidity_stats.reset();

  m_external_climate_sensor_error_cnt = 0;
  m_external_climate_sensor_permanent_fault = false;
  m_external_temperature_stats.reset();
}

////////////////////////////////////////////////////////////////

cmon_climate_sampler::~cmon_climate_sampler(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_climate_sampler::~cmon_climate_sampler\n",
		this->get_name().c_str());
  }
}

////////////////////////////////////////////////////////////////

void cmon_climate_sampler::shutdown(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_climate_sampler::shutdown\n",
		this->get_name().c_str());
  }

  // Initiate a controlled shutdown
  m_shutdown_requested = true;

  this->stop();
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long cmon_climate_sampler::setup(void)
{
  // Perform setup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_climate_sampler::setup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->initialize_climate_sampler();

    if (m_verbose) {
      cmon_io_put("%s : setup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_climate_sampler::setup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_climate_sampler::setup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_climate_sampler::cleanup(void)
{
  // Perform cleanup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_climate_sampler::cleanup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->finalize_climate_sampler();

    if (m_verbose) {
      cmon_io_put("%s : cleanup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_climate_sampler::cleanup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_climate_sampler::cleanup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_climate_sampler::execute(void *arg)
{
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_climate_sampler:execute, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    if (arg) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_BAD_ARGUMENT,
		"Argument not NULL", NULL);
    }

    this->handle_climate_sampler();

    if (m_verbose) {
      cmon_io_put("%s : execute done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Execute OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_climate_sampler::execute, exception:\n%s\n",
		this->get_name().c_str(),
		gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_climate_sampler::execute, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_climate_sampler::initialize_climate_sampler(void)
{
}

////////////////////////////////////////////////////////////////

void cmon_climate_sampler::finalize_climate_sampler(void)
{
}

////////////////////////////////////////////////////////////////

void cmon_climate_sampler::handle_climate_sampler(void)
{
  long rc;

  float int_temperature;
  float int_humidity;
  float ext_temperature;

  bool int_sensor_error;
  bool ext_sensor_error;

  timer climate_timer;

  const clockid_t the_clock = get_clock_id();
  struct timespec t1;
  struct timespec t2;

  // Get start time  
  if (clock_gettime(the_clock, &t1) == -1) {
    THROW_EXP(CMON_LINUX_ERROR, CMON_TIME_ERROR,
	      "Failed to get sampler start time", NULL);
  }
  climate_timer.reset();
  while (!is_stopped()) {
    //////////////////////////////////////////
    // Sample internal and external climate
    //////////////////////////////////////////

    // There is no point continue when permanent
    // fault in internal climate sensor is detected.
    if (m_internal_climate_sensor_permanent_fault) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_INTERNAL_OPERATION_FAILED,
		"Interal climate sensor permanent fault", NULL);
    }
    else {
      this->sample_internal_climate(int_temperature,
				    int_humidity,
				    int_sensor_error); // Sample internal climate
      if (!int_sensor_error) {
	m_internal_temperature_stats.insert(int_temperature);
	m_internal_humidity_stats.insert(int_humidity);
      }
    }

    // We can manage without external climate sensor
    if (!m_external_climate_sensor_permanent_fault) {
      this->sample_external_climate(ext_temperature,
				    ext_sensor_error); // Sample external climate
      if (!ext_sensor_error) {
	m_external_temperature_stats.insert(ext_temperature);
      }
    }

    ///////////////////////////////////
    // Check if time to log climate
    ///////////////////////////////////
    // Compensate for early wake up
    if (climate_timer.get_elapsed_time() >= (CLIMATE_LOG_INTERVAL - 0.010)) {

      // Allocate climate data from pool
      cmon_climate_data *climate_data_p = NULL;
      this->allocate_climate_data(&climate_data_p);
      this->create_climate_data(climate_data_p);  // Create climate data  
      
      // Check if time to quit
      if (m_shutdown_requested) {
	if (climate_data_p) {
	  m_climate_data_queue->deallocate_pool(climate_data_p);
	}
	break;
      }
      else {
	// Put climate data into queue
	rc = m_climate_data_queue->send(climate_data_p);
	if (rc != CMON_CLIMATE_DATA_QUEUE_SUCCESS) {
	  THROW_EXP(CMON_INTERNAL_ERROR, CMON_MSG_QUEUE_ERROR,
		    "Send climate data queue failed", NULL);
	}
      }

      // Prepare new log interval
      climate_timer.reset();
      m_internal_temperature_stats.reset();
      m_internal_humidity_stats.reset();
      m_external_temperature_stats.reset();
    }

    //////////////////////////////////
    // Take it easy until next time
    //////////////////////////////////
    if (get_new_time(&t1, CLIMATE_SAMPLE_INTERVAL, &t2) != DELAY_SUCCESS ) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIME_ERROR,
		"Failed to get sampler next time", NULL);
    }
    if (delay_until(&t2) != DELAY_SUCCESS) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIME_ERROR,
		"Failed to delay sampler until next time", NULL);
    }
    t1.tv_sec = t2.tv_sec;
    t1.tv_nsec = t2.tv_nsec;
  }
}

////////////////////////////////////////////////////////////////

void cmon_climate_sampler::sample_internal_climate(float &temperature,
						   float &humidity,
						   bool &sensor_error)
{
  string sensor_error_info;

  sensor_error = false; // Assume no errors

  // Sample internal climate by reading sensor.
  // To many consecutive errors, and we will assume
  // some kind of permanent fault.
  try {
    temperature = cmon_int_sensor_get_temperature();
    humidity = cmon_int_sensor_get_humidity();
  }
  catch (cmon_exception &cxp) {
    sensor_error = true;
    sensor_error_info = cxp.get_info();
    if (++m_internal_climate_sensor_error_cnt >= MAX_CONSECUTIVE_INTERNAL_CLIMATE_SENSOR_ERRORS) {
      m_internal_climate_sensor_permanent_fault = true;
    }
  }
  catch (...) {
    throw; // Zero tolerance for unexpected exceptions
  }

  // Handle sensor readings
  if (sensor_error) {
    cmon_io_put("%s : *** Warning: internal climate sensor fault => %s\n",
		this->get_name().c_str(),
		sensor_error_info.c_str());
  }
  else {
    m_internal_climate_sensor_error_cnt = 0; // Reset any previous errors
    if (m_verbose) {
      cmon_io_put("%s : internal climate, temp=%+-8.3f, hum=%-8.3f\n",
		  this->get_name().c_str(),
		  temperature,
		  humidity);
    }
  }
}

////////////////////////////////////////////////////////////////

void cmon_climate_sampler::sample_external_climate(float &temperature,
						   bool &sensor_error)
{
  string sensor_error_info;

  sensor_error = false; // Assume no errors

  // Sample external climate by reading sensor.
  // To many consecutive errors, and we will assume
  // some kind of permanent fault.
  try {
    temperature = cmon_ext_sensor_get_temperature();
  }
  catch (cmon_exception &cxp) {
    sensor_error = true;
    sensor_error_info = cxp.get_info();
    if (++m_external_climate_sensor_error_cnt >= MAX_CONSECUTIVE_EXTERNAL_CLIMATE_SENSOR_ERRORS) {
      m_external_climate_sensor_permanent_fault = true;
    }
  }
  catch (...) {
    throw; // Zero tolerance for unexpected exceptions
  }

  // Handle sensor readings
  if (sensor_error) {
    cmon_io_put("%s : *** Warning: external climate sensor fault => %s\n",
		this->get_name().c_str(),
		sensor_error_info.c_str());
  }
  else {
    m_external_climate_sensor_error_cnt = 0; // Reset any previous errors
    if (m_verbose) {
      cmon_io_put("%s : external climate, temp=%+-8.3f\n",
		  this->get_name().c_str(),
		  temperature);
    }
  }
}

////////////////////////////////////////////////////////////////

void cmon_climate_sampler::allocate_climate_data(cmon_climate_data **climate_data)
{
  long rc;
  cmon_climate_data *climate_data_p = NULL;
  unsigned allocate_tries = 100; // Timeout 1s

  do {
    if (m_shutdown_requested) { // Quit if shutdown requested
      break;
    }

    rc = m_climate_data_queue->allocate_pool(climate_data_p);
    if (rc == CMON_CLIMATE_DATA_QUEUE_SUCCESS) {
      break;
    }
    else if (rc == CMON_CLIMATE_DATA_QUEUE_MAX_LIMIT) {      
      cmon_nanosleep(0.01);
      allocate_tries--;
    }
    else {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_MSG_QUEUE_ERROR,
		"Allocate climate data queue memory pool failed", NULL);
    }
  } while (allocate_tries);

  if (!allocate_tries) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIMEOUT_OCCURRED,
	      "Timeout waiting for climate data queue memory in pool", NULL);
  }

  *climate_data = climate_data_p;
}

////////////////////////////////////////////////////////////////

void cmon_climate_sampler::create_climate_data(cmon_climate_data *climate_data)
{
  memset(&climate_data->m_climate_data, 0, sizeof(climate_data->m_climate_data));

  // Get broken down time
  time_t now = time(NULL);
  if ( localtime_r(&now, &climate_data->m_climate_data.time.tstruct) == NULL ) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIME_ERROR,
	      "localtime_r failed", NULL);
  }

  // Internal climate
  if (!m_internal_climate_sensor_permanent_fault) {
    climate_data->m_climate_data.internal_temperature.valid = true;
    climate_data->m_climate_data.internal_temperature.min = m_internal_temperature_stats.get_min();
    climate_data->m_climate_data.internal_temperature.max = m_internal_temperature_stats.get_max();
    climate_data->m_climate_data.internal_temperature.mean = m_internal_temperature_stats.get_mean();
    climate_data->m_climate_data.internal_humidity.valid = true;
    climate_data->m_climate_data.internal_humidity.min = m_internal_humidity_stats.get_min();
    climate_data->m_climate_data.internal_humidity.max = m_internal_humidity_stats.get_max();
    climate_data->m_climate_data.internal_humidity.mean = m_internal_humidity_stats.get_mean();
    
  }
  else {
    climate_data->m_climate_data.internal_temperature.valid = false;
    climate_data->m_climate_data.internal_humidity.valid = false;
  }
  
  // External climate
  if (!m_external_climate_sensor_permanent_fault) {
    climate_data->m_climate_data.external_temperature.valid = true;
    climate_data->m_climate_data.external_temperature.min = m_external_temperature_stats.get_min();
    climate_data->m_climate_data.external_temperature.max = m_external_temperature_stats.get_max();
    climate_data->m_climate_data.external_temperature.mean = m_external_temperature_stats.get_mean();
  }
  else {
    climate_data->m_climate_data.external_temperature.valid = false;
  }
}
