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

#include "cmon_int_sensor.h"
#include "cmon_ext_sensor.h"
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
typedef struct {
  int               sensor_error_cnt;
  bool              sensor_permanent_fault;
  item_stats<float> temperature_stats;
  item_stats<float> humidity_stats;
} CMON_INTERNAL_CLIMATE;

typedef struct {
  int               sensor_error_cnt;
  bool              sensor_permanent_fault;
  item_stats<float> temperature_stats;
} CMON_EXTERNAL_CLIMATE;

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
  CMON_INTERNAL_CLIMATE m_internal_climate;

  // External climate
  CMON_EXTERNAL_CLIMATE m_external_climate[CMON_EXT_MAX_SENSORS];

  void initialize_climate_sampler(void);
  void finalize_climate_sampler(void);
  void handle_climate_sampler(void);

  // Support functions for the internal thread
  void sample_internal_climate(float &temperature,
			       float &humidity,
			       bool &sensor_error);

  void sample_external_climate(CMON_EXT_SENSOR sensor,
			       float &temperature,
			       bool &sensor_error);

  void allocate_climate_data(cmon_climate_data **climate_data);

  void create_climate_data(cmon_climate_data *climate_data);
};

#endif // __CMON_CLIMATE_SAMPLER_H__
