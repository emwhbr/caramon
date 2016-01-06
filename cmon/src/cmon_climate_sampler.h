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

#ifndef __CMON_CLIMATE_SAMPLER_H__
#define __CMON_CLIMATE_SAMPLER_H__

#include <stdint.h>
#include <string>

#include "cmon_internal_climate.h"
#include "cmon_external_climate.h"
#include "cmon_climate_data_queue.h"
#include "thread.h"
#include "item_stats.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_climate_sampler : public thread {
 public:
  cmon_climate_sampler(string thread_name,
		       uint32_t cpu_affinity_mask,
		       int rt_priority,
		       cmon_climate_data_queue *climate_data_queue,
		       bool verbose);

  ~cmon_climate_sampler(void);

  void shutdown(void);

 protected:
  virtual long setup(void);         // Implements pure virtual functions from base class
  virtual long cleanup(void);       // Implements pure virtual functions from base class
  virtual long execute(void *arg);  // Implements pure virtual functions from base class

 private:
  bool m_verbose;

  // Controlled shutdown
  bool m_shutdown_requested;

  // QUEUE           PRODUCER           CONSUMER
  // Data queue      Internal thread    External thread
  cmon_climate_data_queue *m_climate_data_queue;

  // Internal climate
  cmon_internal_climate *m_internal_climate;
  int                   m_internal_climate_sensor_error_cnt;
  bool                  m_internal_climate_sensor_permanent_fault;
  item_stats<float>     m_internal_temperature_stats;
  item_stats<float>     m_internal_humidity_stats;

  // External climate
  cmon_external_climate *m_external_climate;
  int                   m_external_climate_sensor_error_cnt;
  bool                  m_external_climate_sensor_permanent_fault;
  item_stats<float>     m_external_temperature_stats;

  void initialize_climate_sampler(void);
  void finalize_climate_sampler(void);
  void handle_climate_sampler(void);

  // Support functions for the internal thread
  void sample_internal_climate(float &temperature,
			       float &humidity,
			       bool &sensor_error);

  void sample_external_climate(float &temperature,
			       bool &sensor_error);

  void allocate_climate_data(cmon_climate_data **climate_data);

  void create_climate_data(cmon_climate_data *climate_data);
};

#endif // __CMON_CLIMATE_SAMPLER_H__
