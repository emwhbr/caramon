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

#ifndef __CMON_CORE_H__
#define __CMON_CORE_H__

#include <memory>

#include "cmon_alive.h"
#include "cmon_climate_sampler.h"
#include "cmon_climate_logger.h"
#include "cmon_temp_controller.h"
#include "cmon_climate_data_queue.h"
#include "cmon_controller_data_queue.h"
#include "cmon_fallback.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_core {
 public:
  cmon_core(bool fallback,
	    bool disable_disk_log,
	    bool disable_net_log,
	    bool enable_temp_ctrl,
	    bool verbose);
  ~cmon_core(void);
  
  void initialize(void);
  void finalize(void);
  void check_ok(void);
  
 private:
  bool m_fallback;
  bool m_disable_disk_log;
  bool m_disable_net_log;
  bool m_enable_temp_ctrl;
  bool m_verbose;

  // Queues - Normal operation
  cmon_climate_data_queue    *m_climate_data_queue;
  cmon_controller_data_queue *m_controller_data_queue;

  // Threads - Normal operation
  auto_ptr<cmon_alive> m_alive_auto;
  auto_ptr<cmon_climate_sampler> m_climate_sampler_auto;
  auto_ptr<cmon_climate_logger> m_climate_logger_auto;
  auto_ptr<cmon_temp_controller> m_temp_controller_auto;

  // Threads - Fallback operation
  auto_ptr<cmon_fallback> m_fallback_auto;

  // Normal operation
  void setup_climate_control(void);
  void cleanup_climate_control(void);
  void check_climate_control(void);

  // Fallback operation
  void setup_fallback(void);
  void cleanup_fallback(void);
  void check_fallback(void);
};

#endif // __CMON_CORE_H__
