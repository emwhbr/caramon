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
#include "cmon_exception.h"
#include "cmon_utility.h"
#include "cmon_io.h"
#include "cmon_file.h"
#include "shell_cmd.h"
#include "timer.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
// Temperature PID controller
#define TEMP_PID_KP  163.64
#define TEMP_PID_KI   28.05
#define TEMP_PID_KD    0.00

#define TEMP_PID_FILE_SET_VALUE      "/caramon/temp_pid_set_value"
#define TEMP_PID_DEFAULT_SET_VALUE   15.0  // [deg C]
#define TEMP_PID_MIN_SET_VALUE        5.0  // [deg C]
#define TEMP_PID_MAX_SET_VALUE       16.0  // [deg C]

#define TEMP_PID_PERIOD_TIME_SEC   600.0  // [s]
//#define TEMP_PID_PERIOD_TIME_SEC    20.0  // [s] JOE: For testing

#define CONTROLLER_LOG_INTERVAL  TEMP_PID_PERIOD_TIME_SEC

// Remote control of wall outlet (240V) for radiator
#define TX433_CMD          "/caramon/tx433"
#define TX433_CMD_EXIT_OK  0

#define TX433_SYSTEM_CODE  "4"
#define TX433_UNIT_CODE    "3"

#define TX433_ON   "1"
#define TX433_OFF  "0"

#define TX433_MIN_CTRL_TIME  10.0  // [s] Minimum time for activation/deactivation

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

  m_controller_data_queue = controller_data_queue;

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

  this->radiator_off(); // Turn off radiator
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::finalize_temp_controller(void)
{
  this->radiator_off(); // Turn off radiator
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::handle_temp_controller(void)
{
  long rc;

  float temp;
  double radiator_duty;  
  double radiator_on_sec;
  double radiator_off_sec;

  timer controller_timer;

  // Temperature PID controller loop
  controller_timer.reset();
  while (!is_stopped()) {
    // Update PID
    this->check_temp_controller_set_value();
    temp = cmon_int_sensor_get_temperature();
    radiator_duty = m_temp_pid->update((double)temp);

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
      controller_timer.reset();  // Prepare new log interval
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

void cmon_temp_controller::radiator_pulse(bool on,
					  double pulse_time_sec)
{
  timer pulse_timer;

  pulse_timer.reset();
  if (pulse_time_sec > TX433_MIN_CTRL_TIME) {
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
  const string radiator_on_cmd =
    string(TX433_CMD) + " " +
    string(TX433_SYSTEM_CODE) + " " +
    string(TX433_UNIT_CODE) + " " +
    string(TX433_ON);

  this->execute_tx433_cmd(radiator_on_cmd);
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::radiator_off(void)
{
  const string radiator_off_cmd =
    string(TX433_CMD) + " " +
    string(TX433_SYSTEM_CODE) + " " +
    string(TX433_UNIT_CODE) + " " +
    string(TX433_OFF);

  this->execute_tx433_cmd(radiator_off_cmd);
}

////////////////////////////////////////////////////////////////

void cmon_temp_controller::execute_tx433_cmd(const string cmd)
{
  shell_cmd sc_tx433;
  int exit_status;

  // Execute command
  if (sc_tx433.execute(cmd, exit_status) != SHELL_CMD_SUCCESS) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_SHELL_OPERATION_FAILED,
	      "Failed to execute command : %s", cmd.c_str());
  }

  // Check command exit status
  if (exit_status != TX433_CMD_EXIT_OK) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_SHELL_OPERATION_FAILED,
	      "Command failed : %s, exit status : %d",
	      cmd.c_str(), exit_status);
  }
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
  controller_data->m_controller_data.temp_controller.valid = true;
  controller_data->m_controller_data.temp_controller.duty = m_temp_pid->get_output();
  controller_data->m_controller_data.temp_controller.set_value = m_temp_pid->get_command_position();
}
