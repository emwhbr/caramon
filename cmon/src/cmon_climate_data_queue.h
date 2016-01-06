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

#ifndef __CMON_CLIMATE_DATA_QUEUE_H__
#define __CMON_CLIMATE_DATA_QUEUE_H__

#include <string>

#include "cmon_climate_data.h"
#include "msg_fifo_queue.h"
#include "basic_memory_pool.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
// Return codes
#define CMON_CLIMATE_DATA_QUEUE_SUCCESS       0
#define CMON_CLIMATE_DATA_QUEUE_FAILURE      -1
#define CMON_CLIMATE_DATA_QUEUE_WOULD_BLOCK  -2
#define CMON_CLIMATE_DATA_QUEUE_TIMEDOUT     -3
#define CMON_CLIMATE_DATA_QUEUE_MAX_LIMIT    -4

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class cmon_climate_data_queue {

 public:
  cmon_climate_data_queue(void);
  cmon_climate_data_queue(unsigned nr_pool_elements);
  ~cmon_climate_data_queue(void);

  ///// Memory pool operations

  long allocate_pool(cmon_climate_data *&ptr);   // Call before send to queue

  long deallocate_pool(cmon_climate_data *&ptr); // Call after receive from queue

  long size_pool(unsigned &value);

  ///// Queue operations

  long send(const cmon_climate_data *ptr);

  long recv(cmon_climate_data *&ptr,
	    bool wait);

  long recv(cmon_climate_data *&ptr,
	    double timeout_in_sec);

  long size_queue(unsigned &value);

  long clear_queue(void);

 private:
  // The actual queue
  msg_fifo_queue<cmon_climate_data *> m_item_queue;

  // Item pool
  basic_memory_pool<cmon_climate_data> *m_item_pool;
};

#endif // __CMON_CLIMATE_DATA_QUEUE_H__
