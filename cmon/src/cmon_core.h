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
#include "cmon_climate_data_queue.h"

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
  cmon_core(bool disable_disk_log,
	    bool disable_net_log,
	    bool verbose);
  ~cmon_core(void);
  
  void initialize(void);
  void finalize(void);
  void check_ok(void);
  
 private:
  bool m_disable_disk_log;
  bool m_disable_net_log;
  bool m_verbose;

  // Queues
  cmon_climate_data_queue *m_climate_data_queue;

  // Threads
  auto_ptr<cmon_alive> m_alive_auto;
  auto_ptr<cmon_climate_sampler> m_climate_sampler_auto;
  auto_ptr<cmon_climate_logger> m_climate_logger_auto;

  void setup_climate_control(void);
  void cleanup_climate_control(void);
  void check_climate_control(void);
};

#endif // __CMON_CORE_H__
