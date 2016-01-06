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

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define ALIVE_THREAD_START_TIMEOUT    2.0
#define ALIVE_THREAD_EXECUTE_TIMEOUT  2.0
#define ALIVE_THREAD_STOP_TIMEOUT     2.0

#define CLIMATE_SAMPLER_THREAD_START_TIMEOUT    2.0
#define CLIMATE_SAMPLER_THREAD_EXECUTE_TIMEOUT  1.0
#define CLIMATE_SAMPLER_THREAD_STOP_TIMEOUT     10.0

#define CLIMATE_LOGGER_THREAD_START_TIMEOUT    2.0
#define CLIMATE_LOGGER_THREAD_EXECUTE_TIMEOUT  1.0
#define CLIMATE_LOGGER_THREAD_STOP_TIMEOUT     10.0

#define CLIMATE_DATA_QUEUE_INITIAL_NR_ELEMENTS  10

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_core::cmon_core(bool disable_disk_log,
		     bool disable_net_log,
		     bool verbose)
{
  if (verbose) {
    cmon_io_put("cmon_core::cmon_core\n");
  }

  m_disable_disk_log = disable_disk_log;
  m_disable_net_log = disable_net_log;
  m_verbose = verbose;

  // Create queues
  m_climate_data_queue =
    new cmon_climate_data_queue(CLIMATE_DATA_QUEUE_INITIAL_NR_ELEMENTS);
}

////////////////////////////////////////////////////////////////

cmon_core::~cmon_core(void)
{
  if (m_verbose) {
    cmon_io_put("cmon_core::~cmon_core\n");
  }

  delete m_climate_data_queue;
}

////////////////////////////////////////////////////////////////

void cmon_core::initialize(void)
{
  this->setup_climate_control();
}

////////////////////////////////////////////////////////////////

void cmon_core::finalize(void)
{
  this->cleanup_climate_control();
}

////////////////////////////////////////////////////////////////

void cmon_core::check_ok(void)
{
  this->check_climate_control();
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_core::setup_climate_control(void)
{
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

  //////////////////////////////////////////
  // Finalize climate sampler thread
  //////////////////////////////////////////
  if (m_verbose) {
    cmon_io_put("About to finalize climate sampler thread\n");
  }

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

  //////////////////////////////////////////
  // Finalize climate logger thread
  //////////////////////////////////////////
  if (m_verbose) {
    cmon_io_put("About to finalize climate logger thread\n");
  }

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
  try {
    cmon_thread_check_status((thread *)alive);
    cmon_thread_check_status((thread *)climate_sampler);
    cmon_thread_check_status((thread *)climate_logger);
  }
  catch (...) {
    m_alive_auto = auto_ptr<cmon_alive>(alive);
    m_climate_sampler_auto = auto_ptr<cmon_climate_sampler>(climate_sampler);
    m_climate_logger_auto = auto_ptr<cmon_climate_logger>(climate_logger);
    throw;
  }

  // Give back ownership to auto_ptr
   m_alive_auto = auto_ptr<cmon_alive>(alive);
  m_climate_sampler_auto = auto_ptr<cmon_climate_sampler>(climate_sampler);
  m_climate_logger_auto = auto_ptr<cmon_climate_logger>(climate_logger);
}
