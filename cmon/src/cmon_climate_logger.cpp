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
#include <time.h>

#include "cmon_climate_logger.h"
#include "cmon_io.h"
#include "cmon_exception.h"
#include "cmon_utility.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define MAX_CONSECUTIVE_LOG_DISK_ERRORS  10
#define MAX_CONSECUTIVE_LOG_NET_ERRORS   10

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_climate_logger::
cmon_climate_logger(string thread_name,
		    uint32_t cpu_affinity_mask,
		    int rt_priority,
		    cmon_climate_data_queue *climate_data_queue,
		    bool disable_disk_log,
		    bool disable_net_log,
		    bool verbose) : thread(thread_name,
					   cpu_affinity_mask,
					   rt_priority)
{
  if (verbose) {
    cmon_io_put("%s : cmon_climate_logger::cmon_climate_logger\n",
		this->get_name().c_str());
  }

  m_disable_disk_log = disable_disk_log;
  m_disable_net_log = disable_net_log;
  m_verbose = verbose;
  m_shutdown_requested = false;

  m_climate_data_queue = climate_data_queue;

  m_log_disk = new cmon_log_disk(this->get_name() + "-DISK",
				 m_verbose,
				 "/caramon/" + this->get_date_time_prefix() + "_cmon_climate.data");
  m_log_disk_error_cnt = 0;
  m_log_disk_permanent_fault = false;

  m_log_net = new cmon_log_net(this->get_name() + "-NET",
			       m_verbose);
  m_log_net_error_cnt = 0;
  m_log_net_permanent_fault = false;
}

////////////////////////////////////////////////////////////////

cmon_climate_logger::~cmon_climate_logger(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_climate_logger::~cmon_climate_logger\n",
		this->get_name().c_str());
  }
  delete m_log_disk;
  delete m_log_net;
}

////////////////////////////////////////////////////////////////

void cmon_climate_logger::shutdown(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_climate_logger::shutdown\n",
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

long cmon_climate_logger::setup(void)
{
  // Perform setup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_climate_logger::setup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->initialize_climate_logger();

    if (m_verbose) {
      cmon_io_put("%s : setup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_climate_logger::setup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_climate_logger::setup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_climate_logger::cleanup(void)
{
  // Perform cleanup
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_climate_logger::cleanup, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    this->finalize_climate_logger();

    if (m_verbose) {
      cmon_io_put("%s : cleanup done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Setup OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_climate_logger::cleanup, exception:\n%s\n",
		this->get_name().c_str(), gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_climate_logger::cleanup, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long cmon_climate_logger::execute(void *arg)
{
  try {
    if (m_verbose) {
      cmon_io_put("%s : cmon_climate_logger:execute, pid=%u, tid=%u\n",
		  this->get_name().c_str(),
		  (unsigned)get_pid(), (unsigned)get_tid());
    }

    if (arg) {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_BAD_ARGUMENT,
		"Argument not NULL", NULL);
    }

    this->handle_climate_logger();

    if (m_verbose) {
      cmon_io_put("%s : execute done\n", this->get_name().c_str());
    }

    return THREAD_SUCCESS;  // Execute OK
  }
  catch (cmon_exception &gxp) {
    cmon_io_put("%s : cmon_climate_logger::execute, exception:\n%s\n",
		this->get_name().c_str(),
		gxp.get_details().c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    cmon_io_put("%s : cmon_climate_logger::execute, Unexpected exception\n",
		this->get_name().c_str());
    return THREAD_INTERNAL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_climate_logger::initialize_climate_logger(void)
{
  if (!m_disable_disk_log) {
    m_log_disk->initialize();
  }
  if (!m_disable_net_log) {
    m_log_net->initialize();
  }
}

////////////////////////////////////////////////////////////////

void cmon_climate_logger::finalize_climate_logger(void)
{
  if (!m_disable_disk_log) {
    m_log_disk->finalize();
  }
  if (!m_disable_net_log) {
    m_log_net->finalize();
  }
}

////////////////////////////////////////////////////////////////

void cmon_climate_logger::handle_climate_logger(void)
{
  CMON_CLIMATE_DATA climate_data;
  bool climate_data_received;

  while (!is_stopped()) {
    ///////////////////////////////
    // Wait for new climate data
    ///////////////////////////////
    this->recv_climate_data(&climate_data, 1.0, climate_data_received);
    if (!climate_data_received) {
      cmon_nanosleep(0.1);
    }
    else {
      ////////////////////////////
      // Log new climate data
      ////////////////////////////
      if (m_verbose) {
	this->print_climate_data(&climate_data);
      }
      // There is no point continue when permanent
      // fault in disk logger is detected.
      if (!m_disable_disk_log) {
	if (m_log_disk_permanent_fault) {
	  THROW_EXP(CMON_INTERNAL_ERROR, CMON_INTERNAL_OPERATION_FAILED,
		    "Log disk permanent fault", NULL);
	}
	else {
	  this->log_disk(&climate_data); // Log to disk
	}
      }
      
      // We can manage without net logger
      if (!m_disable_net_log) {
	if (!m_log_net_permanent_fault) {
	  this->log_net(&climate_data); // Log to network
	}
      }
    }
  }
}

////////////////////////////////////////////////////////////////

string cmon_climate_logger::get_date_time_prefix(void)
{
  char buffer[40];

  time_t now = time(NULL);
  struct tm *tstruct;

  // Get broken down time
  tstruct = localtime(&now);
  if ( tstruct == NULL ) {
    return "UNKWON_DATE_TIME";
  }

  // Current date/time, format is YYYY-MM-DD-HH:mm:ss
  if (strftime(buffer,
               sizeof(buffer),
               "%Y-%m-%d-%X",
               tstruct) == 0) {
    return "UNKWON_DATE_TIME";
  }

  return buffer;
}

////////////////////////////////////////////////////////////////

void cmon_climate_logger::recv_climate_data(CMON_CLIMATE_DATA *data,
					    double timeout_in_sec,
					    bool &data_received)
{
  long rc;
  cmon_climate_data *climate_data_p = NULL;

  // Get climate data from queue
  rc = m_climate_data_queue->recv(climate_data_p, timeout_in_sec);
  if (rc != CMON_CLIMATE_DATA_QUEUE_SUCCESS) {
    if (rc == CMON_CLIMATE_DATA_QUEUE_TIMEDOUT) {
      data_received = false;
    }
    else {
      THROW_EXP(CMON_INTERNAL_ERROR, CMON_MSG_QUEUE_ERROR,
		"Receive climate data queue failed", NULL);
    }    
  }
  else {
    data_received = true;
    memcpy(data, &climate_data_p->m_climate_data, sizeof(*data));
    m_climate_data_queue->deallocate_pool(climate_data_p);
  }
}

////////////////////////////////////////////////////////////////

void cmon_climate_logger::print_climate_data(const CMON_CLIMATE_DATA *data)
{
  cmon_io_put("%s : ========== NEW DATA ===========\n",
	      this->get_name().c_str());
  
  if (data->internal_temperature.valid) {
    cmon_io_put("%s : internal temp, min=%+-8.3f, max=%+-8.3f, mean=%+-8.3f\n",
		this->get_name().c_str(),
		data->internal_temperature.min,
		data->internal_temperature.max,
		data->internal_temperature.mean);
  }
  else {
    cmon_io_put("%s : internal temp NOT valid\n", this->get_name().c_str());
  }
  if (data->internal_humidity.valid) {
    cmon_io_put("%s : internal hum , min=%+-8.3f, max=%+-8.3f, mean=%+-8.3f\n",
		this->get_name().c_str(),
		data->internal_humidity.min,
		data->internal_humidity.max,
		data->internal_humidity.mean);
  }
  else {
    cmon_io_put("%s : internal hum NOT valid\n", this->get_name().c_str());
  }
  if (data->external_temperature.valid) {
    cmon_io_put("%s : external temp, min=%+-8.3f, max=%+-8.3f, mean=%+-8.3f\n",
		this->get_name().c_str(),
		data->external_temperature.min,
		data->external_temperature.max,
		data->external_temperature.mean);
  }
  else {
    cmon_io_put("%s : external temp NOT valid\n", this->get_name().c_str());
  }
}

////////////////////////////////////////////////////////////////

void cmon_climate_logger::log_disk(const CMON_CLIMATE_DATA *data)
{
  bool log_error;
  string log_error_info;

  log_error = false; // Assume no errors

  // Log climate data to disk
  // To many consecutive errors, and we will assume
  // some kind of permanent fault.
  try {
    m_log_disk->log_climate_data(data);
  }
  catch (cmon_exception &cxp) {
    log_error = true;
    log_error_info = cxp.get_info();
    if (++m_log_disk_error_cnt >= MAX_CONSECUTIVE_LOG_DISK_ERRORS) {
      m_log_disk_permanent_fault = true;
    }
  }
  catch (...) {
    throw; // Zero tolerance for unexpected exceptions
  }

  // Handle sensor readings
  if (log_error) {
    cmon_io_put("%s : *** Warning: log disk fault => %s\n",
		this->get_name().c_str(),
		log_error_info.c_str());
  }
  else {
    m_log_disk_error_cnt = 0; // Reset any previous errors
  }
}

////////////////////////////////////////////////////////////////

void cmon_climate_logger::log_net(const CMON_CLIMATE_DATA *data)
{
  bool log_error;
  string log_error_info;

  log_error = false; // Assume no errors

  // Log climate data to network
  // To many consecutive errors, and we will assume
  // some kind of permanent fault.
  try {
    m_log_net->log_climate_data(data);
  }
  catch (cmon_exception &cxp) {
    log_error = true;
    log_error_info = cxp.get_info();
    if (++m_log_net_error_cnt >= MAX_CONSECUTIVE_LOG_NET_ERRORS) {
      m_log_net_permanent_fault = true;
    }
  }
  catch (...) {
    throw; // Zero tolerance for unexpected exceptions
  }

  // Handle sensor readings
  if (log_error) {
    cmon_io_put("%s : *** Warning: log net fault => %s\n",
		this->get_name().c_str(),
		log_error_info.c_str());
  }
  else {
    m_log_net_error_cnt = 0; // Reset any previous errors
  }
}
