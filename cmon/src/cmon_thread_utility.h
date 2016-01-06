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

#ifndef __CMON_THREAD_UTILITY_H__
#define __CMON_THREAD_UTILITY_H__

#include "thread.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////
extern void cmon_thread_initialize(thread *ct,
				   double ct_start_timeout,
				   double ct_execute_timeout);

extern void cmon_thread_finalize(thread *ct,
				  double ct_stop_timeout);

extern void cmon_thread_check_running(thread *ct);

extern void cmon_thread_check_status(thread *ct);

#endif // __CMON_THREAD_UTILITY_H__
