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

#include "cmon_core.h"
#include "cmon.h"
#include "cmon_io.h"
#include "cmon_thread_utility.h"
#include "cmon_int_sensor.h"
#include "cmon_ext_sensor.h"
#include "cmon_wdt.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define ALIVE_THREAD_START_TIMEOUT    2.0
#define ALIVE_THREAD_EXECUTE_TIMEOUT  2.0
#define ALIVE_THREAD_STOP_TIMEOUT     2.0

#define CLIMATE_SAMPLER_THREAD_START_TIMEOUT    2.0
#define CLIMATE_SAMPLER_THREAD_EXECUTE_TIMEOUT  1.0
#define CLIMATE_SAMPLER_THREAD_STOP_TIMEOUT     10.0

#define CLIMATE_LOGGER_THREAD_START_TIMEOUT    4.0
#define CLIMATE_LOGGER_THREAD_EXECUTE_TIMEOUT  1.0
#define CLIMATE_LOGGER_THREAD_STOP_TIMEOUT     10.0

#define TEMP_CONTROLLER_THREAD_START_TIMEOUT    2.0
#define TEMP_CONTROLLER_THREAD_EXECUTE_TIMEOUT  1.0
#define TEMP_CONTROLLER_THREAD_STOP_TIMEOUT     5.0

#define FALLBACK_THREAD_START_TIMEOUT    2.0
#define FALLBACK_THREAD_EXECUTE_TIMEOUT  2.0
#define FALLBACK_THREAD_STOP_TIMEOUT     2.0

#define CLIMATE_DATA_QUEUE_INITIAL_NR_ELEMENTS     10
#define CONTROLLER_DATA_QUEUE_INITIAL_NR_ELEMENTS  10

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_core::cmon_core(bool fallback,
		     bool disable_disk_log,
		     bool disable_net_log,
		     bool enable_temp_ctrl,
		     bool verbose)
{
  if (verbose) {
    cmon_io_put("cmon_core::cmon_core\n");
  }

  m_fallback = fallback;
  m_disable_disk_log = disable_disk_log;
  m_disable_net_log = disable_net_log;
  m_enable_temp_ctrl = enable_temp_ctrl;
  m_verbose = verbose;

  // Create queues
  if (!m_fallback) {
    m_climate_data_queue =
      new cmon_climate_data_queue(CLIMATE_DATA_QUEUE_INITIAL_NR_ELEMENTS);
    
    m_controller_data_queue =
      new cmon_controller_data_queue(CONTROLLER_DATA_QUEUE_INITIAL_NR_ELEMENTS);
  }

  // Reset thread pointers
  m_alive_auto.reset();
  m_climate_sampler_auto.reset();
  m_climate_logger_auto.reset();
  m_temp_controller_auto.reset();
  m_fallback_auto.reset();
}

////////////////////////////////////////////////////////////////

cmon_core::~cmon_core(void)
{
  if (m_verbose) {
    cmon_io_put("cmon_core::~cmon_core\n");
  }

  if (!m_fallback) {
    delete m_climate_data_queue;
    delete m_controller_data_queue;
  }
}

////////////////////////////////////////////////////////////////

void cmon_core::initialize(void)
{
  if (!m_fallback) {
    this->setup_climate_control();
  }
  else {
    this->setup_fallback();
  }
}

////////////////////////////////////////////////////////////////

void cmon_core::finalize(void)
{
  if (!m_fallback) {
    this->cleanup_climate_control();
  }
  else {
    this->cleanup_fallback();
  }
}

////////////////////////////////////////////////////////////////

void cmon_core::check_ok(void)
{
  if (!m_fallback) {
    this->check_climate_control();
  }
  else {
    this->check_fallback();
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_core::setup_climate_control(void)
{
  //////////////////////////////////////////
  // Initialize climate sensors
  //////////////////////////////////////////
  if (m_verbose) {
    cmon_io_put("About to initialize climate sensors\n");
  }
  cmon_int_sensor_initialize();
  
  // We can manage without the external climate sensors,
  // and we don't want the watchdog to fire due to long timeouts here.
  cmon_wdt_disable();
  try {
    cmon_ext_sensor_initialize(CMON_EXT_SENSOR_1);
  }
  catch (...) {
    cmon_io_put("*** Warning: Failed to initialize external climate sensor 1\n");
  }
  try {
    cmon_ext_sensor_initialize(CMON_EXT_SENSOR_2);
  }
  catch (...) {
    cmon_io_put("*** Warning: Failed to initialize external climate sensor 2\n");
  }
  cmon_wdt_enable(); // Now we wan't the watchdog operational again

  //////////////////////////////////////////
  // Initialize alive thread
  //////////////////////////////////////////
  // Create thread object with garbage collector
  cmon_alive *alive =
    new cmon_alive("ALIVE",
		   CMON_DEFAULT_THREAD_CPU_AFFINITY_MASK,
		   CMON_DEFAULT_THREAD_RT_PRIORITY,
		   m_verbose);
  m_alive_auto = auto_ptr<cmon_alive>(alive);

  if (m_verbose) {
    cmon_io_put("About to initialize alive thread\n");
  }

  // Take back ownership from auto_ptr
  alive = m_alive_auto.release();

  try {
    // Initialize thread object
    cmon_thread_initialize((thread *)alive,
			   ALIVE_THREAD_START_TIMEOUT,
			   ALIVE_THREAD_EXECUTE_TIMEOUT);
  }
  catch (...) {
    m_alive_auto = auto_ptr<cmon_alive>(alive);
    throw;
  }
  
  // Give back ownership to auto_ptr
  m_alive_auto = auto_ptr<cmon_alive>(alive);

  //////////////////////////////////////////
  // Initialize climate sampler thread
  //////////////////////////////////////////
  // Create thread object with garbage collector
  cmon_climate_sampler *climate_sampler =
    new cmon_climate_sampler("CLIMATE-SAMPLER",
			     CMON_DEFAULT_THREAD_CPU_AFFINITY_MASK,
			     CMON_DEFAULT_THREAD_RT_PRIORITY,
			     m_climate_data_queue,
			     m_verbose);
  m_climate_sampler_auto = auto_ptr<cmon_climate_sampler>(climate_sampler);

  if (m_verbose) {
    cmon_io_put("About to initialize climate sampler thread\n");
  }

  // Take back ownership from auto_ptr
  climate_sampler = m_climate_sampler_auto.release();

  try {
    // Initialize thread object
    cmon_thread_initialize((thread *)climate_sampler,
			   CLIMATE_SAMPLER_THREAD_START_TIMEOUT,
			   CLIMATE_SAMPLER_THREAD_EXECUTE_TIMEOUT);
  }
  catch (...) {
    m_climate_sampler_auto = auto_ptr<cmon_climate_sampler>(climate_sampler);
    throw;
  }
  
  // Give back ownership to auto_ptr
  m_climate_sampler_auto = auto_ptr<cmon_climate_sampler>(climate_sampler);

  //////////////////////////////////////////
  // Initialize climate logger thread
  //////////////////////////////////////////
  // Create thread object with garbage collector
  cmon_climate_logger *climate_logger =
    new cmon_climate_logger("CLIMATE-LOGGER",
			    CMON_DEFAULT_THREAD_CPU_AFFINITY_MASK,
			    CMON_DEFAULT_THREAD_RT_PRIORITY,
			    m_climate_data_queue,
			    m_controller_data_queue,
			    m_enable_temp_ctrl,
			    m_disable_disk_log,
			    m_disable_net_log,
			    m_verbose);
  m_climate_logger_auto = auto_ptr<cmon_climate_logger>(climate_logger);

  if (m_verbose) {
    cmon_io_put("About to initialize climate logger thread\n");
  }

  // Take back ownership from auto_ptr
  climate_logger = m_climate_logger_auto.release();

  try {
    // Initialize thread object
    cmon_thread_initialize((thread *)climate_logger,
			   CLIMATE_LOGGER_THREAD_START_TIMEOUT,
			   CLIMATE_LOGGER_THREAD_EXECUTE_TIMEOUT);
  }
  catch (...) {
    m_climate_logger_auto = auto_ptr<cmon_climate_logger>(climate_logger);
    throw;
  }
  
  // Give back ownership to auto_ptr
  m_climate_logger_auto = auto_ptr<cmon_climate_logger>(climate_logger);

  //////////////////////////////////////////////////
  // Initialize temperature controller thread
  //////////////////////////////////////////////////
  if (m_enable_temp_ctrl) {
    // Create thread object with garbage collector
    cmon_temp_controller *temp_controller =
      new cmon_temp_controller("TEMP-CTRL",
			       CMON_DEFAULT_THREAD_CPU_AFFINITY_MASK,
			       CMON_DEFAULT_THREAD_RT_PRIORITY,
			       m_controller_data_queue,
			       m_verbose);
    m_temp_controller_auto = auto_ptr<cmon_temp_controller>(temp_controller);
    
    if (m_verbose) {
      cmon_io_put("About to initialize temperature controller thread\n");
    }
    
  // Take back ownership from auto_ptr
    temp_controller = m_temp_controller_auto.release();
    
    try {
      // Initialize thread object
      cmon_thread_initialize((thread *)temp_controller,
			     TEMP_CONTROLLER_THREAD_START_TIMEOUT,
			     TEMP_CONTROLLER_THREAD_EXECUTE_TIMEOUT);
    }
    catch (...) {
      m_temp_controller_auto = auto_ptr<cmon_temp_controller>(temp_controller);
      throw;
    }
    
    // Give back ownership to auto_ptr
    m_temp_controller_auto = auto_ptr<cmon_temp_controller>(temp_controller);
  }
}

////////////////////////////////////////////////////////////////

void cmon_core::cleanup_climate_control(void)
{
  //////////////////////////////////////////
  // Finalize alive thread
  //////////////////////////////////////////
  if (m_verbose) {
    cmon_io_put("About to finalize alive thread\n");
  }

  // Shutdown thread object
  if (m_alive_auto.get()) {
    m_alive_auto->shutdown();

    // Take back ownership from auto_ptr
    cmon_alive *alive = m_alive_auto.release();
    
    try {
      // Finalize thread object
      cmon_thread_finalize((thread *)alive,
			   ALIVE_THREAD_STOP_TIMEOUT);
    }
    catch (...) {
      m_alive_auto = auto_ptr<cmon_alive>(alive);
      throw;
    }    
    // Give back ownership to auto_ptr
    m_alive_auto = auto_ptr<cmon_alive>(alive);
  }

  //////////////////////////////////////////
  // Finalize climate sampler thread
  //////////////////////////////////////////
  if (m_verbose) {
    cmon_io_put("About to finalize climate sampler thread\n");
  }

  if (m_climate_sampler_auto.get()) {
    // Shutdown thread object
    m_climate_sampler_auto->shutdown();
    
    // Take back ownership from auto_ptr
    cmon_climate_sampler *climate_sampler = m_climate_sampler_auto.release();
    
    try {
      // Finalize thread object
      cmon_thread_finalize((thread *)climate_sampler,
			   CLIMATE_SAMPLER_THREAD_STOP_TIMEOUT);
    }
    catch (...) {
      m_climate_sampler_auto = auto_ptr<cmon_climate_sampler>(climate_sampler);
      throw;
    }  
    // Give back ownership to auto_ptr
    m_climate_sampler_auto = auto_ptr<cmon_climate_sampler>(climate_sampler);
  }

  //////////////////////////////////////////
  // Finalize climate logger thread
  //////////////////////////////////////////
  if (m_verbose) {
    cmon_io_put("About to finalize climate logger thread\n");
  }

  if (m_climate_logger_auto.get()) {
    // Shutdown thread object
    m_climate_logger_auto->shutdown();
    
    // Take back ownership from auto_ptr
    cmon_climate_logger *climate_logger = m_climate_logger_auto.release();
    
    try {
      // Finalize thread object
      cmon_thread_finalize((thread *)climate_logger,
			   CLIMATE_LOGGER_THREAD_STOP_TIMEOUT);
    }
    catch (...) {
      m_climate_logger_auto = auto_ptr<cmon_climate_logger>(climate_logger);
      throw;
    }  
    // Give back ownership to auto_ptr
    m_climate_logger_auto = auto_ptr<cmon_climate_logger>(climate_logger);
  }

  /////////////////////////////////////////////
  // Finalize temperature controller thread
  /////////////////////////////////////////////
  if (m_enable_temp_ctrl) {
    if (m_verbose) {
      cmon_io_put("About to finalize temperature controller thread\n");
    }
    
    if (m_temp_controller_auto.get()) {
      // Shutdown thread object
      m_temp_controller_auto->shutdown();
      
      // Take back ownership from auto_ptr
      cmon_temp_controller *temp_controller = m_temp_controller_auto.release();
      
      try {
	// Finalize thread object
	cmon_thread_finalize((thread *)temp_controller,
			     TEMP_CONTROLLER_THREAD_STOP_TIMEOUT);
      }
      catch (...) {
	m_temp_controller_auto = auto_ptr<cmon_temp_controller>(temp_controller);
	throw;
      }    
      // Give back ownership to auto_ptr
      m_temp_controller_auto = auto_ptr<cmon_temp_controller>(temp_controller);
    }
  }

  //////////////////////////////////////////
  // Finalize climate sensors
  //////////////////////////////////////////
  if (m_verbose) {
    cmon_io_put("About to finalize climate sensors\n");
  }
  cmon_int_sensor_finalize();

  // We can manage without external climate sensors
  try {
    cmon_ext_sensor_finalize(CMON_EXT_SENSOR_1);
  }
  catch (...) {
    cmon_io_put("*** Warning: Failed to initialize external climate sensor 1");
  }
   try {
    cmon_ext_sensor_finalize(CMON_EXT_SENSOR_2);
  }
  catch (...) {
    cmon_io_put("*** Warning: Failed to initialize external climate sensor 2");
  }
}

////////////////////////////////////////////////////////////////

void cmon_core::check_climate_control(void)
{
  //////////////////////////////
  // Check status of threads
  //////////////////////////////
  // Take back ownership from auto_ptr
  cmon_alive *alive = m_alive_auto.release();
  cmon_climate_sampler *climate_sampler = m_climate_sampler_auto.release();
  cmon_climate_logger *climate_logger = m_climate_logger_auto.release();
  cmon_temp_controller *temp_controller = m_temp_controller_auto.release();
  try {
    cmon_thread_check_status((thread *)alive);
    cmon_thread_check_status((thread *)climate_sampler);
    cmon_thread_check_status((thread *)climate_logger);
    if (m_enable_temp_ctrl) {
      cmon_thread_check_status((thread *)temp_controller);
    }
  }
  catch (...) {
    m_alive_auto = auto_ptr<cmon_alive>(alive);
    m_climate_sampler_auto = auto_ptr<cmon_climate_sampler>(climate_sampler);
    m_climate_logger_auto = auto_ptr<cmon_climate_logger>(climate_logger);
    m_temp_controller_auto = auto_ptr<cmon_temp_controller>(temp_controller);
    throw;
  }

  // Give back ownership to auto_ptr
  m_alive_auto = auto_ptr<cmon_alive>(alive);
  m_climate_sampler_auto = auto_ptr<cmon_climate_sampler>(climate_sampler);
  m_climate_logger_auto = auto_ptr<cmon_climate_logger>(climate_logger);
  m_temp_controller_auto = auto_ptr<cmon_temp_controller>(temp_controller);
}

////////////////////////////////////////////////////////////////

void cmon_core::setup_fallback(void)
{
  //////////////////////////////////////////
  // Initialize fallback thread
  //////////////////////////////////////////
  // Create thread object with garbage collector
  cmon_fallback *fallback =
    new cmon_fallback("FALLBACK",
		      CMON_DEFAULT_THREAD_CPU_AFFINITY_MASK,
		      CMON_DEFAULT_THREAD_RT_PRIORITY,
		      m_verbose);
  m_fallback_auto = auto_ptr<cmon_fallback>(fallback);
  
  if (m_verbose) {
    cmon_io_put("About to initialize fallback thread\n");
  }

  // Take back ownership from auto_ptr
  fallback = m_fallback_auto.release();

  try {
    // Initialize thread object
    cmon_thread_initialize((thread *)fallback,
			   FALLBACK_THREAD_START_TIMEOUT,
			   FALLBACK_THREAD_EXECUTE_TIMEOUT);
  }
  catch (...) {
    m_fallback_auto = auto_ptr<cmon_fallback>(fallback);
    throw;
  }  
  // Give back ownership to auto_ptr
  m_fallback_auto = auto_ptr<cmon_fallback>(fallback);
}

////////////////////////////////////////////////////////////////

void cmon_core::cleanup_fallback(void)
{
  //////////////////////////////////////////
  // Finalize fallback thread
  //////////////////////////////////////////
  if (m_verbose) {
    cmon_io_put("About to finalize fallback thread\n");
  }
  
  if (m_fallback_auto.get()) {
    // Shutdown thread object
    m_fallback_auto->shutdown();
    
    // Take back ownership from auto_ptr
    cmon_fallback *fallback = m_fallback_auto.release();
    
    try {
      // Finalize thread object
      cmon_thread_finalize((thread *)fallback,
			   FALLBACK_THREAD_STOP_TIMEOUT);
    }
    catch (...) {
      m_fallback_auto = auto_ptr<cmon_fallback>(fallback);
      throw;
    }    
    // Give back ownership to auto_ptr
    m_fallback_auto = auto_ptr<cmon_fallback>(fallback);
  }
}

////////////////////////////////////////////////////////////////

void cmon_core::check_fallback(void)
{
  //////////////////////////////
  // Check status of threads
  //////////////////////////////
  // Take back ownership from auto_ptr
  cmon_fallback *fallback = m_fallback_auto.release();
  try {
    cmon_thread_check_status((thread *)fallback);
  }
  catch (...) {
    m_fallback_auto = auto_ptr<cmon_fallback>(fallback);
    throw;
  }

  // Give back ownership to auto_ptr
  m_fallback_auto = auto_ptr<cmon_fallback>(fallback);
}
