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

#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#include "cmon_utility.h"
#include "cmon_exception.h"
#include "cmon.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_mutex_lock(pthread_mutex_t *mutex)
{
  if (pthread_mutex_lock(mutex)) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_MUTEX_LOCK_FAILED,
              "Mutex lock failed", NULL);
  }
}

////////////////////////////////////////////////////////////////

void cmon_mutex_unlock(pthread_mutex_t *mutex)
{
  if (pthread_mutex_unlock(mutex)) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_MUTEX_UNLOCK_FAILED,
              "Mutex unlock failed", NULL);
  }
}

////////////////////////////////////////////////////////////////

long cmon_get_my_pid(void)
{
  return (long)getpid();
}

////////////////////////////////////////////////////////////////

long cmon_get_my_thread_id(void)
{
  return (long)syscall(SYS_gettid);
}

////////////////////////////////////////////////////////////////

void cmon_nanosleep(double timesec)
{
  int rc;
  struct timespec ts;

  if ( timesec < 0.0 ) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_BAD_ARGUMENT,
              "timesec negative(%f)", timesec);
  }

  // Extracts seconds and nanoseconds
  ts.tv_sec  = (time_t) timesec;
  ts.tv_nsec = (long int)( (timesec - (time_t)timesec) * 1000000000.0 );

  // Do nanosleep and protect from being interrupted
  // by any signals (EINTR)
  do
    {
      rc = nanosleep(&ts, &ts);
    }
  while ( rc && (errno == EINTR) );

  if (rc) {
    THROW_EXP(CMON_LINUX_ERROR, CMON_TIME_ERROR,
              "nanosleep failed(%f)", timesec);
  }
}

////////////////////////////////////////////////////////////////

void cmon_sched_yield(void)
{
  sched_yield(); // The Linux implementation
                 // sched_yield() always succeeds
}

////////////////////////////////////////////////////////////////

void cmon_flush_to_disk(void)
{
  sync();
}

////////////////////////////////////////////////////////////////

void cmon_delete_file(const char *pathname)
{
  int rc;

  rc = unlink(pathname);
  if (rc == -1) {
    THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
              "unlink failed for file(%s)", pathname);
  }
}

////////////////////////////////////////////////////////////////

void cmon_print_memory(const char *banner,
		       const unsigned char *start,
		       unsigned length)
{
  unsigned col;
  unsigned i = 0;
  
  printf("%s\n", banner);

  if (!length) {
    printf("No data\n");
    return;
  } 

  // Iterate through the rows, which will be 16 bytes of memory wide
  for (unsigned row=0; (i + 1) <= length; row++) {

    printf("0x%04x : ", row * 16);

    // Print HEX representation
    for (col=0; col<16; col++) {
      i = row * 16 + col;  // Calculate the current index
            
      if (col == 8) {      // Divides a row of 16 into two columns of 8
        printf(" ");
      }      
      if (i < length) {
        printf("%02x", start[i]); // Print the hex value if the current index is in range     
      }
      else {
        printf("  ");             // Print a blank if the current index is past the end
      }
      printf(" "); // Print a space to keep the values separate
    }    
    printf(" ");   // Create a vertial seperator between HEX and ASCII representations
    
    // Print ASCII representation
    for (col=0; col < 16; col++) {
      i = row * 16 + col;  // Calculate the current index
      
      if(col==8) {         // Divides a row of 16 into two coumns of 8
        printf("  ");
      }
        
      // Print the value if it is in range
      if (i < length) {
        // Print the ascii value if applicable
        if ((start[i] > 0x20) && (start[i] < 0x7F)) {
          printf("%c", start[i]);
        }       
        else {
          printf("."); // Print a period if the value is not printable
        }
      }
      else {
        break;         // Nothing else to print, so break out of this for loop
      }
    }
    
    printf("\n");     // Create a new row
  }
}
