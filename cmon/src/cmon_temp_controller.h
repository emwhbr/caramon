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

#ifndef __CMON_TEMP_CONTROLLER_H__
#define __CMON_TEMP_CONTROLLER_H__

#include "cmon_controller_data_queue.h"
#include "cmon_power_switch.h"
#include "thread.h"
#include "item_stats.h"
#include "pid_ctrl.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
typedef struct {
  item_stats<float> duty_stats;
  item_stats<float> set_value_stats;
} CMON_CONTROLLER;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_temp_controller : public thread {
 public:
  cmon_temp_controller(string thread_name,
		       uint32_t cpu_affinity_mask,
		       int rt_priority,
		       cmon_controller_data_queue *ccontroller_data_queue,
		       bool verbose);

  ~cmon_temp_controller(void);

  void shutdown(void);

 protected:
  virtual long setup(void);         // Implements pure virtual functions from base class
  virtual long cleanup(void);       // Implements pure virtual functions from base class
  virtual long execute(void *arg);  // Implements pure virtual functions from base class

 private:
  bool m_verbose;

  // Controlled shutdown
  bool m_shutdown_requested;

  // Contols wall outlet (230V) for radiator
  cmon_power_switch *m_radiator_switch;

  // QUEUE           PRODUCER           CONSUMER
  // Data queue      Internal thread    External thread
  cmon_controller_data_queue *m_controller_data_queue;

  // Controller
  CMON_CONTROLLER m_controller;

  // Internal climate
  int  m_previous_temp;
  int  m_internal_climate_sensor_error_cnt;
  bool m_internal_climate_sensor_permanent_fault;

  // Temperature PID controller
  pid_ctrl *m_temp_pid;

  void initialize_temp_controller(void);
  void finalize_temp_controller(void);
  void handle_temp_controller(void);

  // Support functions for the internal thread
  void check_temp_controller_set_value(void);

  void sample_internal_temperature(float &temperature,
				   bool &sensor_error);

  void radiator_pulse(bool on,
		      double pulse_time_sec);

  void radiator_on(void);

  void radiator_off(void);

  void allocate_controller_data(cmon_controller_data **controller_data);

  void create_controller_data(cmon_controller_data *controller_data);
};

#endif // __CMON_TEMP_CONTROLLER_H__
