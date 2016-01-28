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

#ifndef __CMON_H__
#define __CMON_H__

#include <signal.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Error codes
/////////////////////////////////////////////////////////////////////////////
// CMON internal error codes
#define CMON_NO_ERROR                      0
#define CMON_BAD_ARGUMENT                  1
#define CMON_MUTEX_LOCK_FAILED             2
#define CMON_MUTEX_UNLOCK_FAILED           3
#define CMON_TIMEOUT_OCCURRED              4
#define CMON_TIME_ERROR                    5
#define CMON_THREAD_OPERATION_FAILED       6
#define CMON_THREAD_STATE_NOT_OK           7
#define CMON_THREAD_STATUS_NOT_OK          8
#define CMON_FILE_OPERATION_FAILED         9
#define CMON_SHELL_OPERATION_FAILED       10
#define CMON_SIGNAL_OPERATION_FAILED      11
#define CMON_SOCKET_OPERATION_FAILED      12
#define CMON_SOCKET_CONNECTION_CLOSED     13
#define CMON_MSG_QUEUE_ERROR              14
#define CMON_CLIMATE_SENSOR_ERROR         15
#define CMON_GPIO_ERROR                   16
#define CMON_INTERNAL_OPERATION_FAILED    17
#define CMON_UNEXPECTED_EXCEPTION         18

// Error source values
typedef enum {CMON_INTERNAL_ERROR,
              CMON_LINUX_ERROR} CMON_ERROR_SOURCE;

/////////////////////////////////////////////////////////////////////////////
//               Internal exceptions (non-critical)
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Signals
/////////////////////////////////////////////////////////////////////////////
// Signal used for terminate
#define CMON_SIG_TERMINATE   SIGTERM

/////////////////////////////////////////////////////////////////////////////
//               Misc definitions
/////////////////////////////////////////////////////////////////////////////
#define CMON_DEFAULT_THREAD_CPU_AFFINITY_MASK  0xffffffff // Allow all CPU's
#define CMON_DEFAULT_THREAD_RT_PRIORITY        -1         // Use default priority

// Remote control of wall outlet (230V) for radiator
#define CMON_TX433_SYSTEM_CODE  4
#define CMON_TX433_UNIT_CODE    3

#endif // __CMON_H__
