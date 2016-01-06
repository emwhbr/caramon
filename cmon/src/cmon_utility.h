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

#ifndef __CMON_UTILITY_H__
#define __CMON_UTILITY_H__

#include <pthread.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////
extern void cmon_mutex_lock(pthread_mutex_t *mutex);
extern void cmon_mutex_unlock(pthread_mutex_t *mutex);

extern long cmon_get_my_pid(void);
extern long cmon_get_my_thread_id(void);

extern void cmon_nanosleep(double timesec);

extern void cmon_sched_yield(void);

extern void cmon_flush_to_disk(void);

extern void cmon_delete_file(const char *pathname);

extern void cmon_print_memory(const char *banner,
			      const unsigned char *start,
			      unsigned length);

#endif // __CMON_UTILITY_H__
