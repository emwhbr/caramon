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

#ifndef __MSG_FIFO_QUEUE_H__
#define __MSG_FIFO_QUEUE_H__

#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <queue>
#include <deque>

// Use best available clock identifer
#if defined CLOCK_MONOTONIC
#define CLK_ID CLOCK_MONOTONIC
#elif defined CLOCK_REALTIME
#warning CLOCK_MONOTONIC not defined, using CLOCK_REALTIME
#define CLK_ID CLOCK_REALTIME
#else
#error CLOCK_xxx not defined, abort
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Return codes
#define MFQ_SUCCESS        0
#define MFQ_MUTEX_ERROR   -1
#define MFQ_SIGNAL_ERROR  -2
#define MFQ_WOULD_BLOCK   -3
#define MFQ_TIMEDOUT      -4
#define MFQ_BAD_ARGUMENT  -5
#define MFQ_TIME_ERROR    -6

// Flag values
#define MFQ_FLAG_NONBLOCK  0x01

#define NSEC_PER_SEC  1000000000L

static inline void tsnorm(struct timespec *ts);

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

template <class T> 
class msg_fifo_queue {

 public:
  msg_fifo_queue(void);
  ~msg_fifo_queue(void);

  long push(const T &msg);

  long pop(T &msg, unsigned flag);

  long pop_timed(T &msg, double timeout_in_sec);

  long nr_elements(unsigned &value);

  long clear(void);

 private:
  queue<T, deque<T> > m_queue; // Underlaying container is deque
  pthread_mutex_t     m_queue_mutex;
  pthread_cond_t      m_queue_condv;  
  pthread_condattr_t  m_queue_condattr;

  long get_future_time(const struct timespec *old_time,
		       double diff_in_sec,
		       struct timespec *future_time);
};

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

template <class T>
msg_fifo_queue<T>::msg_fifo_queue(void)
{
  pthread_mutex_init(&m_queue_mutex, NULL); // Use default mutex attributes
  
  // Use default condition attributes,
  // but change the clock attribute to best available 
  pthread_condattr_init(&m_queue_condattr); 
  pthread_condattr_setclock(&m_queue_condattr, CLK_ID);
  pthread_cond_init(&m_queue_condv, &m_queue_condattr);
}

////////////////////////////////////////////////////////////////

template <class T>
msg_fifo_queue<T>::~msg_fifo_queue(void)
{
  pthread_mutex_destroy(&m_queue_mutex);
  pthread_cond_destroy(&m_queue_condv);
  pthread_condattr_destroy(&m_queue_condattr);
}

////////////////////////////////////////////////////////////////

template <class T>
long msg_fifo_queue<T>::push(const T &msg)
{
  // Lockdown queue
  if (pthread_mutex_lock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  // Copy element to end of queue,
  // oldest element is first in queue
  m_queue.push(msg);

  // Notify waiting thread
  if (pthread_cond_signal(&m_queue_condv)) {
    pthread_mutex_unlock(&m_queue_mutex);
    return MFQ_SIGNAL_ERROR;
  }

  // Lockup queue
  if (pthread_mutex_unlock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  return MFQ_SUCCESS;
}

////////////////////////////////////////////////////////////////

template <class T>
long msg_fifo_queue<T>::pop(T &msg, unsigned flag)
{
  // Lockdown queue
  if (pthread_mutex_lock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  // Check if queue is empty
  if ( m_queue.empty() && (flag & MFQ_FLAG_NONBLOCK) ) {  
    // Lockup queue
    if (pthread_mutex_unlock(&m_queue_mutex)) {
      return MFQ_MUTEX_ERROR;
    }
    return MFQ_WOULD_BLOCK;
  }

  // While queue is empty, make calling thread wait
  while ( m_queue.empty() ) {
    if (pthread_cond_wait(&m_queue_condv, &m_queue_mutex)) {
      pthread_mutex_unlock(&m_queue_mutex);
      return MFQ_SIGNAL_ERROR;
    }
  }

  // Copy element from beginning of queue (oldest element)
  msg = m_queue.front();

  // Remove this element from queue
  m_queue.pop();

  // Lockup queue
  if (pthread_mutex_unlock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  return MFQ_SUCCESS;
}

////////////////////////////////////////////////////////////////

template <class T>
long msg_fifo_queue<T>::pop_timed(T &msg, double timeout_in_sec)
{
  long rc;
  struct timespec now_time;
  struct timespec future_time;

  // Check arguments
  if (timeout_in_sec < 0.0) {
    return MFQ_BAD_ARGUMENT;
  }

  // Get now time
  if ( clock_gettime(CLK_ID, &now_time) ) {
    return MFQ_TIME_ERROR;
  }

  // Calculate timeout time in the future
  rc = get_future_time(&now_time, timeout_in_sec, &future_time);
  if ( rc != MFQ_SUCCESS ) {
    return rc;
  }

  // Lockdown queue
  if (pthread_mutex_lock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  // While queue is empty, make calling thread wait
  // until data arrives or timeout occurrs
  while ( m_queue.empty() ) {
    rc = pthread_cond_timedwait(&m_queue_condv,
				&m_queue_mutex,
				&future_time);
    if (rc) {
      pthread_mutex_unlock(&m_queue_mutex);
      if (rc == ETIMEDOUT) {
	return MFQ_TIMEDOUT;
      }
      else {
	return MFQ_SIGNAL_ERROR;
      }
    }
  }

  // Copy element from beginning of queue (oldest element)
  msg = m_queue.front();

  // Remove this element from queue
  m_queue.pop();

  // Lockup queue
  if (pthread_mutex_unlock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  return MFQ_SUCCESS;
}

////////////////////////////////////////////////////////////////

template <class T>
long msg_fifo_queue<T>::nr_elements(unsigned &value)
{
  // Lockdown queue
  if (pthread_mutex_lock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  value = m_queue.size();

  // Lockup queue
  if (pthread_mutex_unlock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  return MFQ_SUCCESS;
}

////////////////////////////////////////////////////////////////

template <class T>
long msg_fifo_queue<T>::clear(void)
{
  // Lockdown queue
  if (pthread_mutex_lock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  // Remove all elements from queue
  while (!m_queue.empty()) {
    m_queue.pop();
  }

  // Lockup queue
  if (pthread_mutex_unlock(&m_queue_mutex)) {
    return MFQ_MUTEX_ERROR;
  }

  return MFQ_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

static inline void tsnorm(struct timespec *ts)
{
   while (ts->tv_nsec >= NSEC_PER_SEC) {
      ts->tv_nsec -= NSEC_PER_SEC;
      ts->tv_sec++;
   }
}

////////////////////////////////////////////////////////////////

template <class T>
long msg_fifo_queue<T>::get_future_time(const struct timespec *old_time,
					double diff_in_sec,
					struct timespec *future_time)
{
  long nr_sec;
  long nr_nsec;

  // Check arguments
  if ( (!old_time) || (!future_time) ) {
    return MFQ_BAD_ARGUMENT;
  }
  if ( diff_in_sec < 0.0 ) {
    return MFQ_BAD_ARGUMENT; // Negative deltas not supported
  }

  future_time->tv_sec  = old_time->tv_sec;
  future_time->tv_nsec = old_time->tv_nsec;

  // Calculate future time
  nr_sec  = (long) diff_in_sec;
  nr_nsec = (diff_in_sec - (double)nr_sec) * (double)NSEC_PER_SEC;

  future_time->tv_sec  += nr_sec;
  future_time->tv_nsec += nr_nsec;

  tsnorm(future_time);

  return MFQ_SUCCESS;
}

#endif // __MSG_FIFO_QUEUE_H__
