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

#include "cmon_climate_data_queue.h"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_climate_data_queue::cmon_climate_data_queue(void)
{
  m_item_pool = new basic_memory_pool<cmon_climate_data>(0);
}

////////////////////////////////////////////////////////////////

cmon_climate_data_queue::cmon_climate_data_queue(unsigned nr_pool_elements)
{
  m_item_pool = new basic_memory_pool<cmon_climate_data>(nr_pool_elements);
}

////////////////////////////////////////////////////////////////

cmon_climate_data_queue::~cmon_climate_data_queue(void)
{
  delete m_item_pool;
}

////////////////////////////////////////////////////////////////

long cmon_climate_data_queue::allocate_pool(cmon_climate_data *&ptr)
{
  long rc;
  void *free_space;

  rc = m_item_pool->allocate(&free_space);
  if (rc != BMP_SUCCESS) {
    if (rc == BMP_MAX_LIMIT) {
      return CMON_CLIMATE_DATA_QUEUE_MAX_LIMIT;
    }
    return CMON_CLIMATE_DATA_QUEUE_FAILURE;
  }
  ptr = new (free_space) cmon_climate_data();

  return CMON_CLIMATE_DATA_QUEUE_SUCCESS;
}

////////////////////////////////////////////////////////////////

long cmon_climate_data_queue::deallocate_pool(cmon_climate_data *&ptr)
{
  if (m_item_pool->deallocate(ptr) != BMP_SUCCESS) {
    return CMON_CLIMATE_DATA_QUEUE_FAILURE;
  }

  return CMON_CLIMATE_DATA_QUEUE_SUCCESS;
}

////////////////////////////////////////////////////////////////

long cmon_climate_data_queue::size_pool(unsigned &value)
{
  return this->m_item_pool->nr_elements(value);
}

////////////////////////////////////////////////////////////////

long cmon_climate_data_queue::send(const cmon_climate_data *ptr)
{
  if (m_item_queue.push((cmon_climate_data *)ptr) != MFQ_SUCCESS) {
    return CMON_CLIMATE_DATA_QUEUE_FAILURE;
  }

  return CMON_CLIMATE_DATA_QUEUE_SUCCESS;
}

////////////////////////////////////////////////////////////////

long cmon_climate_data_queue::recv(cmon_climate_data *&ptr,
				   bool wait)
{
  long rc;
  unsigned flag = (wait ? 0 : MFQ_FLAG_NONBLOCK);

  rc = m_item_queue.pop(ptr, flag);
  if (rc != MFQ_SUCCESS) {
    if (rc == MFQ_WOULD_BLOCK) { 
      return CMON_CLIMATE_DATA_QUEUE_WOULD_BLOCK;
    }
    else {
      return CMON_CLIMATE_DATA_QUEUE_FAILURE;
    }
  }

  return CMON_CLIMATE_DATA_QUEUE_SUCCESS;
}

////////////////////////////////////////////////////////////////

long cmon_climate_data_queue::recv(cmon_climate_data *&ptr,
				   double timeout_in_sec)
{
  long rc;

  rc = m_item_queue.pop_timed(ptr, timeout_in_sec);
  if (rc != MFQ_SUCCESS) {
    if (rc == MFQ_TIMEDOUT) { 
      return CMON_CLIMATE_DATA_QUEUE_TIMEDOUT;
    }
    else {
      return CMON_CLIMATE_DATA_QUEUE_FAILURE;
    }
  }

  return CMON_CLIMATE_DATA_QUEUE_SUCCESS;
}

////////////////////////////////////////////////////////////////

long cmon_climate_data_queue::size_queue(unsigned &value)
{
  if (m_item_queue.nr_elements(value) != MFQ_SUCCESS) {
    return CMON_CLIMATE_DATA_QUEUE_FAILURE;
  }

  return CMON_CLIMATE_DATA_QUEUE_SUCCESS;
}

////////////////////////////////////////////////////////////////

long cmon_climate_data_queue::clear_queue(void)
{
  unsigned queue_size;
  cmon_climate_data *item_ptr = NULL;

  // Free all memory, pointed to by pointers in queue
  if (this->size_queue(queue_size) != CMON_CLIMATE_DATA_QUEUE_SUCCESS) {
    return CMON_CLIMATE_DATA_QUEUE_FAILURE;
  }
  while (queue_size)  {    
    if (this->recv(item_ptr, true) != CMON_CLIMATE_DATA_QUEUE_SUCCESS) {
      return CMON_CLIMATE_DATA_QUEUE_FAILURE;
    }

    // Free memory pointed to by this elements
    if (m_item_pool->deallocate(item_ptr) != BMP_SUCCESS) {
      return CMON_CLIMATE_DATA_QUEUE_FAILURE;
    }

    if (this->size_queue(queue_size) != CMON_CLIMATE_DATA_QUEUE_SUCCESS) {
      return CMON_CLIMATE_DATA_QUEUE_FAILURE;
    }
  }

  return CMON_CLIMATE_DATA_QUEUE_SUCCESS;
}
