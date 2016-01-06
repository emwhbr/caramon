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

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "signal_support.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define NSEC_PER_SEC  1000000000L

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

static long create_signal_set(const SIGNAL_SET sig_set,
			      sigset_t *ss)
{
  int rc;

  // Basic check of input set
  if (sig_set.size() == 0) {
    return SIGNAL_SUPPORT_BAD_ARGUMENT;
  }

  // Create signal set
  sigemptyset(ss);
  for (SIGNAL_SET::iterator it=sig_set.begin(); it != sig_set.end(); ++it) {
    rc = sigaddset(ss, *it);
    if (rc == -1) {
      return SIGNAL_SUPPORT_INVALID_SIGNAL;
    }
  }
  return SIGNAL_SUPPORT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long sig_sup_define_signal_handler(int sig,
				   void (*handler)(int))
{
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags   = SA_RESTART;
  sa.sa_handler = handler;

  if (sigaction(sig, &sa, NULL) == -1) {
    if (errno == EINVAL) {
      return SIGNAL_SUPPORT_INVALID_SIGNAL;
    }
    else {
      return SIGNAL_SUPPORT_FAILURE;
    }
  }
  return SIGNAL_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sig_sup_send_signal_self(int sig)
{
  // Send signal to calling process
  if (kill(getpid(), sig) == -1) {
    if (errno == EINVAL) {
      return SIGNAL_SUPPORT_INVALID_SIGNAL;
    }
    else {
      return SIGNAL_SUPPORT_FAILURE;
    }
  }
  return SIGNAL_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sig_sup_mask(const SIGNAL_SET sig_set)
{
  long lrc;
  int rc;
  sigset_t ss;

  // Create signal set to mask
  lrc = create_signal_set(sig_set, &ss);
  if (lrc != SIGNAL_SUPPORT_SUCCESS) {
    return lrc;
  }

  // Mask signals in set
  rc = pthread_sigmask(SIG_BLOCK, &ss, NULL);
  if (rc == -1) {
    if (errno == EINVAL) {
      return SIGNAL_SUPPORT_INVALID_SIGNAL;
    }
    else {
      return SIGNAL_SUPPORT_FAILURE;
    }
  }
  return SIGNAL_SUPPORT_SUCCESS;
}


////////////////////////////////////////////////////////////////

long sig_sup_send_signal(int sig,
			 pthread_t thread)
{
  int rc;

  rc = pthread_kill(thread, sig);
  if (rc == -1) {
    if (rc == EINVAL) {
      return SIGNAL_SUPPORT_INVALID_SIGNAL;
    }
    else {
      return SIGNAL_SUPPORT_FAILURE;
    }
  }
  return SIGNAL_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sig_sup_wait_signal(const SIGNAL_SET sig_set,
			 bool wait,
			 int &sig)
{
  long lrc;
  int rc;
  sigset_t ss;

  // Create signal set to wait for
  lrc = create_signal_set(sig_set, &ss);
  if (lrc != SIGNAL_SUPPORT_SUCCESS) {
    return lrc;
  }

  // Wait for any signal in set
  if (wait) {
    rc = sigwait(&ss, &sig); // Wait forever
    if (rc != 0) {
      return SIGNAL_SUPPORT_FAILURE;
    }
  }
  else {
    const struct timespec wait_time = {0, 0};
    siginfo_t si;
    rc = sigtimedwait(&ss, &si, &wait_time); // Return immediately
    if (rc == -1) {
      if (errno == EINVAL) {
	return SIGNAL_SUPPORT_INVALID_SIGNAL;
      }
      else if (errno == EAGAIN) {
	return SIGNAL_SUPPORT_WOULD_BLOCK;
      }
      else {
	return SIGNAL_SUPPORT_FAILURE;
      }
    }
    sig = rc;
  }
  return SIGNAL_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sig_sup_wait_signal_timed(const SIGNAL_SET sig_set,
			       double timeout_in_sec,
			       int &sig)
{
  long lrc;
  int rc;
  struct timespec wait_time;
  sigset_t ss;
  siginfo_t si;

  // Check arguments
  if (timeout_in_sec < 0.0) {
    return SIGNAL_SUPPORT_BAD_ARGUMENT;
  }

  // Create signal set to wait for
  lrc = create_signal_set(sig_set, &ss);
  if (lrc != SIGNAL_SUPPORT_SUCCESS) {
    return lrc;
  }

  // Create wait interval
  wait_time.tv_sec = (long)timeout_in_sec;
  wait_time.tv_nsec = (timeout_in_sec - (long)timeout_in_sec) * (double)NSEC_PER_SEC;

  // Wait for any signal in set (using timeout)
  rc = sigtimedwait(&ss, &si, &wait_time);
  if (rc == -1) {
    if (errno == EINVAL) {
      return SIGNAL_SUPPORT_INVALID_SIGNAL;
    }
    else if (errno == EAGAIN) {
      return SIGNAL_SUPPORT_TIMEDOUT;
    }
    else {
      return SIGNAL_SUPPORT_FAILURE;
    }
  }

  // Return caught signal to caller
  sig = si.si_signo;

  return SIGNAL_SUPPORT_SUCCESS;
}
