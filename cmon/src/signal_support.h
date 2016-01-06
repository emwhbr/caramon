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

#ifndef __SIGNAL_SUPPORT_H__
#define __SIGNAL_SUPPORT_H__

#include <signal.h>

#include <set>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Return codes
#define SIGNAL_SUPPORT_SUCCESS          0
#define SIGNAL_SUPPORT_BAD_ARGUMENT    -1
#define SIGNAL_SUPPORT_INVALID_SIGNAL  -2
#define SIGNAL_SUPPORT_WOULD_BLOCK     -3
#define SIGNAL_SUPPORT_TIMEDOUT        -4
#define SIGNAL_SUPPORT_FAILURE         -5

/////////////////////////////////////////////////////////////////////////////
//               Definition of support types
/////////////////////////////////////////////////////////////////////////////
typedef set<int> SIGNAL_SET;

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////

extern long sig_sup_define_signal_handler(int sig,
					  void (*handler)(int));

extern long sig_sup_send_signal_self(int sig);

extern long sig_sup_mask(const SIGNAL_SET sig_set);

extern long sig_sup_send_signal(int sig,
				pthread_t thread);

extern long sig_sup_wait_signal(const SIGNAL_SET sig_set,
				bool wait,
				int &sig);

extern long sig_sup_wait_signal_timed(const SIGNAL_SET sig_set,
				      double timeout_in_sec,
				      int &sig);

#endif // __SIGNAL_SUPPORT_H__
