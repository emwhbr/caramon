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

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include <errno.h>

#include "thread.h"
#include "delay.h"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

thread::thread(string thread_name,
	       uint32_t cpu_affinity_mask,
	       int rt_priority)
{
  m_thread_name = thread_name;
  m_cpu_affinity_mask = cpu_affinity_mask;
  m_rt_priority = rt_priority;

  // Init semaphores that controls thread
  sem_init(&m_sem_release,  0, 0); // Initial value is busy
  sem_init(&m_sem_complete, 0, 0); // Initial value is busy

  // Init condition variable/mutex for 'thread done'
  pthread_condattr_init(&m_condattr_thread_done);

  pthread_condattr_setclock(&m_condattr_thread_done,
			    get_clock_id());

  pthread_cond_init(&m_cond_thread_done,
		    &m_condattr_thread_done);

  pthread_mutex_init(&m_mutex_thread_done,
		     NULL); // Use default mutex attributes

  init_members(); 
}

////////////////////////////////////////////////////////////////

thread::~thread(void)
{
  sem_destroy(&m_sem_release);
  sem_destroy(&m_sem_complete);
  pthread_mutex_destroy(&m_mutex_thread_done);
  pthread_cond_destroy(&m_cond_thread_done);
  pthread_condattr_destroy(&m_condattr_thread_done);
}

////////////////////////////////////////////////////////////////

long thread::start(void *p_arg)
{
  int rc;

  // Check if already started
  if ( (m_state != THREAD_STATE_NOT_STARTED)  &&
       (m_state != THREAD_STATE_DONE) ) {
    return THREAD_WRONG_STATE;
  }

  // Get pointer to user data
  // NOTE! Make sure caller of this function doesn't delete the argument
  //       in ANY way. This includes running out of scope with automatic 
  //       variables.
  arg(p_arg);

  // Assure that semaphores are busy so that next 'sem_wait' will hang
  do {
    sched_yield();
    rc = sem_trywait(&m_sem_release);
  } while ( !rc );
  if ( rc && (errno != EAGAIN) ) {
    return THREAD_SEMAPHORE_ERROR;
  }
  do {
    sched_yield();
    rc = sem_trywait(&m_sem_complete);
  } while ( !rc );
  if ( rc && (errno != EAGAIN) ) {
    return THREAD_SEMAPHORE_ERROR;
  }

  init_members();

  // Create thread
  rc = pthread_create(&m_thread, NULL, thread::entry_point, this);
  if ( rc ) {
    return THREAD_PTHREAD_ERROR;
  }

  return THREAD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long thread::release(void)
{
  int rc;

  // Check if ready to be released
  if ( (m_state != THREAD_STATE_STARTED) &&
       (m_state != THREAD_STATE_SETUP_DONE) ) {    

    return THREAD_WRONG_STATE;
  }

  // Release thread
  rc = sem_post(&m_sem_release);
  if ( rc ) {
    return THREAD_SEMAPHORE_ERROR;
  }

  return THREAD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long thread::stop(void)
{
  // Check if ready to be stopped
  if ( (m_state != THREAD_STATE_SETUP_DONE) &&
       (m_state != THREAD_STATE_EXECUTING)  &&
       (m_state != THREAD_STATE_STOPPED)  &&
       (m_state != THREAD_STATE_DONE) ) {
    return THREAD_WRONG_STATE;
  }

  m_stop = true;

  return THREAD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long thread::complete(void)
{
  int rc;

  // Check if ready to be completed
  if ( (m_state != THREAD_STATE_SETUP_DONE) &&
       (m_state != THREAD_STATE_EXECUTING)  &&
       (m_state != THREAD_STATE_STOPPED)  &&
       (m_state != THREAD_STATE_DONE) ) {
    return THREAD_WRONG_STATE;
  }

  // Complete thread
  rc = sem_post(&m_sem_complete);
  if ( rc ) {
    return THREAD_SEMAPHORE_ERROR;
  }

  return THREAD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long thread::wait(void)
{
  int rc;

  // Check if ready to wait
  if ( (m_state != THREAD_STATE_EXECUTING) &&
       (m_state != THREAD_STATE_DONE) ) {

    return THREAD_WRONG_STATE;
  }

  // Join thread
  if (m_thread) {
    rc = pthread_join(m_thread, NULL);
    if ( rc ) {
      return THREAD_PTHREAD_ERROR;
    }
    m_thread = 0; // Join a thread already joined is bad
  }

  return THREAD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long thread::wait_timed(double timeout_in_sec)
{
  int rc;

  // Check if ready to wait
  if ( (m_state != THREAD_STATE_SETUP_DONE) &&
       (m_state != THREAD_STATE_EXECUTING) &&
       (m_state != THREAD_STATE_STOPPED) &&
       (m_state != THREAD_STATE_DONE) ) {

    return THREAD_WRONG_STATE;
  }

  struct timespec t1;
  struct timespec t2;

  if ( clock_gettime(get_clock_id(), &t1) ) {
    return THREAD_TIME_ERROR;
  }
  if ( get_new_time(&t1, timeout_in_sec, &t2) != DELAY_SUCCESS ) {
    return THREAD_TIME_ERROR;
  }

  if (pthread_mutex_lock(&m_mutex_thread_done)) {
    return THREAD_MUTEX_ERROR;
  }

  // Check if thread already done
  if (m_state != THREAD_STATE_DONE) {

    // Wait for thread using timeout
    rc = pthread_cond_timedwait(&m_cond_thread_done,
				&m_mutex_thread_done,
				&t2);
    if ( rc ) {
      pthread_mutex_unlock(&m_mutex_thread_done);
      return THREAD_PTHREAD_ERROR;
    }
  }

  if (pthread_mutex_unlock(&m_mutex_thread_done)) {
    return THREAD_MUTEX_ERROR;
  }

  // Join thread
  return wait();
}

////////////////////////////////////////////////////////////////

string thread::get_name(void)
{
  return m_thread_name;
}

////////////////////////////////////////////////////////////////

pthread_t thread::get_thread(void)
{
  return m_thread;
}

////////////////////////////////////////////////////////////////

pid_t thread::get_tid(void)
{
  return m_tid;
}

////////////////////////////////////////////////////////////////

pid_t thread::get_pid(void)
{
  return m_pid;
}

////////////////////////////////////////////////////////////////

unsigned thread::get_exe_cnt(void)
{
  return m_exe_cnt;
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void thread::run(void *p_arg)
{
  ///////////////////////////////////////////
  // This function executes as a thread
  ///////////////////////////////////////////  

  m_tid = syscall(SYS_gettid);
  m_pid = getpid();  

  // Manipulate thread propertites
  if (set_thread_cpu_affinity() != THREAD_SUCCESS) {
    m_status |= THREAD_STATUS_START_FAILED;
    return;
  }
  if (set_thread_rt_priority() != THREAD_SUCCESS) {
    m_status |= THREAD_STATUS_START_FAILED;
    return;
  }
  if (set_thread_name() != THREAD_SUCCESS) {
    m_status |= THREAD_STATUS_START_FAILED;
    return;
  }

  m_state = THREAD_STATE_STARTED;  

  /////////////////////////////
  // Setup
  /////////////////////////////
  try {
    // Call virtual function, implemented in derived class
    if (setup() != THREAD_SUCCESS) {
      m_status |= THREAD_STATUS_SETUP_FAILED;
    }       
    m_state = THREAD_STATE_SETUP_DONE;

    // Wait until thread is released
    int rc = sem_wait(&m_sem_release);
    if ( rc != 0 ) {
      m_status |= THREAD_STATUS_SETUP_FAILED;
    }    
  }
  catch (...) {
    m_status |= THREAD_STATUS_SETUP_FAILED;
  }

  /////////////////////////////
  // Execute
  /////////////////////////////
  try {
    if (m_state == THREAD_STATE_SETUP_DONE) {

      m_state = THREAD_STATE_EXECUTING;

      // Call virtual function, implemented in derived class
      if (execute(p_arg) != THREAD_SUCCESS) {
	m_status |= THREAD_STATUS_EXECUTE_FAILED;
      }
    }
  }
  catch (...) {
    m_status |= THREAD_STATUS_EXECUTE_FAILED;
  }
  
  /////////////////////////////
  // Cleanup
  /////////////////////////////
  try {
    m_state = THREAD_STATE_STOPPED;

    // Wait until thread is completed
    int rc = sem_wait(&m_sem_complete);
    if ( rc != 0 ) {
      m_status |= THREAD_STATUS_CLEANUP_FAILED;
    } 

    // Call virtual function, implemented in derived class
    if (cleanup() != THREAD_SUCCESS) {
      m_status |= THREAD_STATUS_CLEANUP_FAILED;
    }
  }
  catch (...) {
    m_status |= THREAD_STATUS_CLEANUP_FAILED;
  }

  /////////////////////////////
  // Done
  /////////////////////////////
  try {
    if (pthread_mutex_lock(&m_mutex_thread_done)) {
      m_status |= THREAD_STATUS_DONE_FAILED;
    }

    m_state = THREAD_STATE_DONE;

    if (pthread_mutex_unlock(&m_mutex_thread_done)) {
      m_status |= THREAD_STATUS_DONE_FAILED;
    }
    
    if (pthread_cond_broadcast(&m_cond_thread_done)) {
      m_status |= THREAD_STATUS_DONE_FAILED;
    }    
  }
  catch (...) {
    m_status |= THREAD_STATUS_DONE_FAILED;
  }
}

////////////////////////////////////////////////////////////////

void *thread::entry_point(void *p_this)
{
  thread *the_thread = (thread *)p_this;

  the_thread->run( the_thread->arg() );

  return NULL;
}

////////////////////////////////////////////////////////////////

void thread::update_exe_cnt(void)
{
  m_exe_cnt++;
}

////////////////////////////////////////////////////////////////

bool thread::is_stopped(void)
{
  return m_stop;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void thread::init_members(void)
{
  m_state   = THREAD_STATE_NOT_STARTED;
  m_status  = THREAD_STATUS_OK;
  m_arg     = 0;
  m_thread  = 0;
  m_tid     = 0;
  m_pid     = 0;
  m_exe_cnt = 0;
  m_stop    = false;
}

////////////////////////////////////////////////////////////////

long thread::set_thread_name(void)
{
  int rc;

  string trunc_name = m_thread_name;

  // Truncate name to restricted length (16 = 15 + null)
  if (trunc_name.length() > 15) {
    trunc_name = trunc_name.substr(0, 15);
  }

  rc = pthread_setname_np(m_thread,
			  trunc_name.c_str());
  if (rc) {
    return THREAD_PTHREAD_ERROR;
  }

  return THREAD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long thread::set_thread_cpu_affinity(void)
{
  int rc;
  cpu_set_t cpuset;

  // Set the CPU affinity mask of the thread
  if (m_cpu_affinity_mask) {

    // Create CPU set
    CPU_ZERO(&cpuset);
    for (int i=0 ; i < 32; i++) {
      if ( ((1 << i) & m_cpu_affinity_mask) != 0  ) {
	CPU_SET(i, &cpuset);
      }
    }

    // Set CPU affinity
    rc = pthread_setaffinity_np(m_thread, sizeof(cpu_set_t), &cpuset);
    if (rc) {
      return THREAD_PTHREAD_ERROR;
    }
  }

  return THREAD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long thread::set_thread_rt_priority(void)
{
  int rc;
  int policy;
  struct sched_param params;

  if (m_rt_priority > 0) {

    // Get current scheduling policy and parameters
    rc = pthread_getschedparam(m_thread, &policy, &params);
    if (rc) {
      return THREAD_PTHREAD_ERROR;
    }
    
    params.sched_priority = m_rt_priority; // Update rt prio
    
    // Set new scheduling policy and parameters
    rc = pthread_setschedparam(m_thread, SCHED_FIFO, &params);
    if (rc) {
      return THREAD_PTHREAD_ERROR;
    }
  }

  return THREAD_SUCCESS;
}
