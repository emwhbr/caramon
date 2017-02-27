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

#include <stdlib.h>
#include <string.h>

#include "cmon_temp_controller.h"
#include "cmon_int_sensor.h"
#include "cmon.h"
#include "cmon_exception.h"
#include "cmon_utility.h"
#include "cmon_io.h"
#include "cmon_file.h"
#include "cmon_power_switch_ssr.h"
#include "timer.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
// Temperature PID controller (Ziegler-Nichols)
#define TEMP_PID_KP  211.76
#define TEMP_PID_KI   12.10
#define TEMP_PID_KD  926.45

#define TEMP_PID_FILE_SET_VALUE      "/caramon/temp_pid_set_value"
#define TEMP_PID_DEFAULT_SET_VALUE   19.0  // [deg C]
#define TEMP_PID_MIN_SET_VALUE        5.0  // [deg C]
#define TEMP_PID_MAX_SET_VALUE       20.0  // [deg C]

#define TEMP_PID_PERIOD_TIME_SEC       60.0   // [s]
#define CONTROLLER_LOG_INTERVAL   (5 * 60.0)  // [s]

#define MAX_CONSECUTIVE_INTERNAL_CLIMATE_SENSOR_ERRORS  2

#define RADIATOR_MIN_CTRL_TIME  10.0  // [s]
                                      // Minimum time for activation/deactivation
                                      // of wall outlet for radiator 

#define PIN_RADIATOR_CTRL  GPIO_P1_15  // Controls radiator via SSR K1

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_temp_controller::
cmon_temp_controller(string thread_name,
		     uint32_t cpu_affinity_mask,
		     int rt_priority,
		     cmon_controller_data_queue *controller_data_queue,
		     bool verbose) : thread(thread_name,
					    cpu_affinity_mask,
					    rt_priority)
{
  if (verbose) {
    cmon_io_put("%s : cmon_temp_controller::cmon_temp_controller\n",
		this->get_name().c_str());
  }

  m_verbose = verbose;
  m_shutdown_requested = false;

  m_radiator_switch = new cmon_power_switch_ssr(string("RADIATOR-CTRL"),
						PIN_RADIATOR_CTRL,
						m_verbose);

  m_controller_data_queue = controller_data_queue;

  m_controller.duty_stats.reset();
  m_controller.set_value_stats.reset();

  m_previous_temp = TEMP_PID_DEFAULT_SET_VALUE;
  m_internal_climate_sensor_error_cnt = 0;
  m_internal_climate_sensor_permanent_fault = false;

  m_temp_pid = new pid_ctrl(TEMP_PID_DEFAULT_SET_VALUE,
			    TEMP_PID_KP,
			    TEMP_PID_KI,
			    TEMP_PID_KD,
			    PID_CTRL_DIRECT);
}

////////////////////////////////////////////////////////////////

cmon_temp_controller::~cmon_temp_controller(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_temp_controller::~cmon_temp_controller\n",
		this->get_name().c_str());
  }
  delete m_radiator_switch;
  delete m_temp_pid;
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::shutdown(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_temp_controller::shutdown\n",
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

long cmon_temp_controller::setup(void)
{
  // Perform setup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_temp_controller::setup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->initialize_temp_controller();

    if (m_verbose) {
      cmon_io_put("%s : setup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_temp_controller::setup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_temp_controller::setup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_temp_controller::cleanup(void)
{
  // Perform cleanup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_temp_controller::cleanup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->finalize_temp_controller();

    if (m_verbose) {
      cmon_io_put("%s : cleanup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_temp_controller::cleanup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_temp_controller::cleanup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_temp_controller::execute(void *arg)
{
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_temp_controller:execute, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    if (arg) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_BAD_ARGUMENT,
		"Argument not NULL", NULL);
    }

    this->handle_temp_controller();

    if (m_verbose) {
      cmon_io_put("%s : execute done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Execute OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_temp_controller::execute, exception:\n%s\n",
		this->get_name().c_str(),
		gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_temp_controller::execute, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_temp_controller::initialize_temp_controller(void)
{
  // Initialize temperature PID controller
  m_temp_pid->set_output_limits(0.0, 100.0); // Duty = 0 ..100%
  m_temp_pid->set_command_position(TEMP_PID_DEFAULT_SET_VALUE);
  this->check_temp_controller_set_value();

  // Initialize power switch (Only necessary for SSR controlled power switch)
  cmon_power_switch_ssr *ssr_switch = dynamic_cast<cmon_power_switch_ssr*>(m_radiator_switch);
  if (ssr_switch) {
    ssr_switch->initialize();
  }
  this->radiator_off(); // Turn off radiator
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::finalize_temp_controller(void)
{
  this->radiator_off(); // Turn off radiator

  // Finalize power switch (Only necessary for SSR controlled power switch)
  cmon_power_switch_ssr *ssr_switch = dynamic_cast<cmon_power_switch_ssr*>(m_radiator_switch);
  if (ssr_switch) {
    ssr_switch->finalize();
  }
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::handle_temp_controller(void)
{
  long rc;

  bool int_sensor_error;
  float temp;
  double radiator_duty;  
  double radiator_on_sec;
  double radiator_off_sec;

  timer controller_timer;

  // Temperature PID controller loop
  controller_timer.reset();
  while (!is_stopped()) {

    this->check_temp_controller_set_value();

    // There is no point continue when permanent
    // fault in internal climate sensor is detected.
    if (m_internal_climate_sensor_permanent_fault) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_INTERNAL_OPERATION_FAILED,
		"Interal climate sensor permanent fault", NULL);
    }
    else {
      this->sample_internal_temperature(temp, int_sensor_error);      
      if (int_sensor_error) {
	temp = m_previous_temp;  // Use last known good temperature
      }
      m_previous_temp = temp;
    }

    // Update PID
    radiator_duty = m_temp_pid->update((double)temp);

    m_controller.duty_stats.insert((float)m_temp_pid->get_output());
    m_controller.set_value_stats.insert((float)m_temp_pid->get_command_position());

    // Apply new output
    radiator_on_sec = TEMP_PID_PERIOD_TIME_SEC * radiator_duty / 100.0;
    radiator_off_sec = TEMP_PID_PERIOD_TIME_SEC - radiator_on_sec;

    if (m_verbose) {
      cmon_io_put("%s : duty=%-8.1f, on=%-8.2f, off=%-8.2f, set=%+-8.3f, temp=%+-8.3f\n",
		  this->get_name().c_str(),
		  radiator_duty, radiator_on_sec, radiator_off_sec,
		  m_temp_pid->get_command_position(), temp);
    }

    // Turn radiator ON
    this->radiator_pulse(true, radiator_on_sec);

    // Turn radiator OFF
    this->radiator_pulse(false, radiator_off_sec);

    // Check if time to log controller data, compensate for early wakeup
    if (controller_timer.get_elapsed_time() >= (CONTROLLER_LOG_INTERVAL - 0.010)) {
      // Allocate controller data from pool
      cmon_controller_data *controller_data_p = NULL;
      this->allocate_controller_data(&controller_data_p);
      this->create_controller_data(controller_data_p);  // Create controller data  
      
      // Check if time to quit
      if (m_shutdown_requested) {
	if (controller_data_p) {
	  m_controller_data_queue->deallocate_pool(controller_data_p);
	}
	break;
      }
      else {
	// Put controller data into queue
	rc = m_controller_data_queue->send(controller_data_p);
	if (rc != CMON_CONTROLLER_DATA_QUEUE_SUCCESS) {
	  THROW_EXP(CMON_INTERNAL_ERROR, CMON_MSG_QUEUE_ERROR,
		    "Send controller data queue failed", NULL);
	}
      }

      // Prepare new log interval
      controller_timer.reset();
      m_controller.duty_stats.reset();
      m_controller.set_value_stats.reset();
    }
  }
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::check_temp_controller_set_value(void)
{
  cmon_file set_value_file(TEMP_PID_FILE_SET_VALUE);
  char buffer[64];
  double set_value;

  // Check if a set value for temperature has been
  // written to the file. This value shall only be
  // accepted if within bounds for PID controller.
  if (set_value_file.exists()) {
    set_value_file.open_file(CMON_FILE_READONLY);
    set_value_file.read_file((uint8_t *)buffer, sizeof(buffer), false);
    set_value = atof(buffer);
    set_value_file.close_file();
    if ( (set_value <= TEMP_PID_MAX_SET_VALUE) &&
	 (set_value >= TEMP_PID_MIN_SET_VALUE) ) {
      m_temp_pid->set_command_position(set_value); // Apply new set value
      if (m_verbose) {
	cmon_io_put("%s : New temperature set value=%+-8.3f\n",
		    this->get_name().c_str(), set_value);
      }
    }
    cmon_delete_file(TEMP_PID_FILE_SET_VALUE); // Delete file to avoid
                                               // unnecessary access
  }
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::sample_internal_temperature(float &temperature,
						       bool &sensor_error)
{
  string sensor_error_info;

  sensor_error = false; // Assume no errors

  // Sample internal climate by reading sensor.
  // To many consecutive errors, and we will assume
  // some kind of permanent fault.
  try {
    temperature = cmon_int_sensor_get_temperature();
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
    cmon_io_put("%s : *** Warning: internal climate sensor, fault => %s\n",
		this->get_name().c_str(),
		sensor_error_info.c_str());
  }
  else {
    m_internal_climate_sensor_error_cnt = 0; // Reset any previous errors
    if (m_verbose) {
      cmon_io_put("%s : internal climate, temp=%+-8.3f\n",
		  this->get_name().c_str(),
		  temperature);
    }
  }
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::radiator_pulse(bool on,
					  double pulse_time_sec)
{
  timer pulse_timer;

  pulse_timer.reset();
  if (pulse_time_sec > RADIATOR_MIN_CTRL_TIME) {
    if (on) {
      this->radiator_on();
    }
    else {
      this->radiator_off();
    }
    if (m_verbose) {
      cmon_io_put("%s : radiator %s\n",
		  this->get_name().c_str(),
		  (on ? "ON" : "OFF"));
    }
  }
  else {
    if (m_verbose) {
      cmon_io_put("%s : radiator %s skipped\n",
		  this->get_name().c_str(),
		  (on ? "ON" : "OFF"));
    }
  }
  while (pulse_timer.get_elapsed_time() < pulse_time_sec) {
    if (is_stopped()) {
      break;
    }
    cmon_nanosleep(0.05); // Take it easy
  }
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::radiator_on(void)
{
  m_radiator_switch->switch_on();
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::radiator_off(void)
{
  m_radiator_switch->switch_off();
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::allocate_controller_data(cmon_controller_data **controller_data)
{
  long rc;
  cmon_controller_data *controller_data_p = NULL;
  unsigned allocate_tries = 100; // Timeout 1s

  do {
    if (m_shutdown_requested) { // Quit if shutdown requested
      break;
    }

    rc = m_controller_data_queue->allocate_pool(controller_data_p);
    if (rc == CMON_CONTROLLER_DATA_QUEUE_SUCCESS) {
      break;
    }
    else if (rc == CMON_CONTROLLER_DATA_QUEUE_MAX_LIMIT) {      
      cmon_nanosleep(0.01);
      allocate_tries--;
    }
    else {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_MSG_QUEUE_ERROR,
		"Allocate controller data queue memory pool failed", NULL);
    }
  } while (allocate_tries);

  if (!allocate_tries) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIMEOUT_OCCURRED,
	      "Timeout waiting for controller data queue memory in pool", NULL);
  }

  *controller_data = controller_data_p;
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::create_controller_data(cmon_controller_data *controller_data)
{
  memset(&controller_data->m_controller_data, 0, sizeof(controller_data->m_controller_data));

  // Get broken down time
  time_t now = time(NULL);
  if ( localtime_r(&now, &controller_data->m_controller_data.time.tstruct) == NULL ) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIME_ERROR,
	      "localtime_r failed", NULL);
  }

  // Controller data
  controller_data->m_controller_data.duty.valid = true;
  controller_data->m_controller_data.duty.min = m_controller.duty_stats.get_min();
  controller_data->m_controller_data.duty.max = m_controller.duty_stats.get_max();
  controller_data->m_controller_data.duty.mean = m_controller.duty_stats.get_mean();
  controller_data->m_controller_data.set_value.valid = true;
  controller_data->m_controller_data.set_value.min = m_controller.set_value_stats.get_min();
  controller_data->m_controller_data.set_value.max = m_controller.set_value_stats.get_max();
  controller_data->m_controller_data.set_value.mean = m_controller.set_value_stats.get_mean();
}
