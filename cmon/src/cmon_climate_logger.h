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

#ifndef __CMON_CLIMATE_LOGGER_H__
#define __CMON_CLIMATE_LOGGER_H__

#include <stdint.h>
#include <string>

#include "cmon_climate_data_queue.h"
#include "cmon_log_disk.h"
#include "cmon_log_net.h"
#include "thread.h"

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
class cmon_climate_logger : public thread {
 public:
  cmon_climate_logger(string thread_name,
		      uint32_t cpu_affinity_mask,
		      int rt_priority,
		      cmon_climate_data_queue *climate_data_queue,
		      bool disable_disk_log,
		      bool disable_net_log,
		      bool verbose);

  ~cmon_climate_logger(void);

  void shutdown(void);

 protected:
  virtual long setup(void);         // Implements pure virtual functions from base class
  virtual long cleanup(void);       // Implements pure virtual functions from base class
  virtual long execute(void *arg);  // Implements pure virtual functions from base class

 private:
  bool m_disable_disk_log;
  bool m_disable_net_log;
  bool m_verbose;

  // Controlled shutdown
  bool m_shutdown_requested;

  // QUEUE                   PRODUCER           CONSUMER
  // Climate data queue      External thread    Internal thread
  cmon_climate_data_queue *m_climate_data_queue;

  // Disk logger
  cmon_log_disk *m_log_disk;
  int           m_log_disk_error_cnt;
  bool          m_log_disk_permanent_fault;

  // Network logger
  cmon_log_net *m_log_net;
  int          m_log_net_error_cnt;
  bool         m_log_net_permanent_fault;

  void initialize_climate_logger(void);
  void finalize_climate_logger(void);
  void handle_climate_logger(void);

  string get_date_time_prefix(void);

  // Support functions for the internal thread
  void recv_climate_data(CMON_CLIMATE_DATA *data,
			 double timeout_in_sec,
			 bool &data_received);

  void print_climate_data(const CMON_CLIMATE_DATA *data);

  void log_disk(const CMON_CLIMATE_DATA *data);

  void log_net(const CMON_CLIMATE_DATA *data);  
};

#endif // __CMON_CLIMATE_LOGGER_H__
