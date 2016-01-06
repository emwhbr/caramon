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
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>

#include "cmon_io.h"
#include "cmon_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Static members
/////////////////////////////////////////////////////////////////////////////
cmon_io* cmon_io::m_instance = NULL;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_io::~cmon_io(void)
{
  pthread_mutex_destroy(&m_write_mutex);
}

////////////////////////////////////////////////////////////////

cmon_io* cmon_io::instance(void)
{
  if (!m_instance) {
    m_instance = new cmon_io;
    static auto_ptr<cmon_io> m_auto = auto_ptr<cmon_io>(m_instance);
  }
  return m_instance;
}

////////////////////////////////////////////////////////////////

void cmon_io::initialize(string logfile)
{
  int rc;

  m_logfile = logfile;

  // Open logfile (if any shall be used)
  if (!m_logfile.empty()) {
    rc = open(m_logfile.c_str(), 
	      O_WRONLY | O_CREAT,
	      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (rc == -1) {
      THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
		"open failed, logfile(%s)", m_logfile.c_str());
    }
    m_fd = rc;
    
    // Move to end of file
    if ( lseek(m_fd, 0, SEEK_END) == -1 ) {
      close(m_fd);
      m_fd = -1;
      THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
		"lseek failed, logfile (%s)", m_logfile.c_str());
    }
  }
}

////////////////////////////////////////////////////////////////

void cmon_io::finalize(void)
{
  int rc;

  // Close logfile (if any was used)
  if (m_fd != -1) {
    rc = close(m_fd);
    if (rc == -1) {
      THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
		"close failed, logfile (%s)", m_logfile.c_str());
    }
  }
}
	
////////////////////////////////////////////////////////////////

void cmon_io::put(const char *format, ...)
{  
  // Retrieve any additional arguments for the format string
  char info_buffer[4096];
  va_list info_args;
  va_start(info_args, format);
  vsprintf(info_buffer, format, info_args);
  va_end(info_args);
	
  // Lockdown write operation
  pthread_mutex_lock(&m_write_mutex);
  
  try {
    // Decorate message with date and time prefix
    char prefix[40];
    this->get_date_time_prefix(prefix, sizeof(prefix));
    
    // Create the timestamped message
    string the_message(prefix); 
    the_message.append(info_buffer);
    
    // Write message to STDOUT
    printf("%s", the_message.c_str());
    
    // Write message to logfile (if any shall be used)
    if (m_fd != -1) {
      this->write_all(m_fd,
		      (uint8_t *)the_message.c_str(),
		      the_message.length());
    }
    
    // Lockup write operation
    pthread_mutex_unlock(&m_write_mutex);
  }
  catch (...) {
    pthread_mutex_unlock(&m_write_mutex);
    throw;
  }
}

////////////////////////////////////////////////////////////////

void cmon_io::put(string message)
{
  this->put(message.c_str());
}

////////////////////////////////////////////////////////////////

void cmon_io::progress_bar(int percent)
{
  // NOTE!
  // This function is not thread safe.

  const char bars[] = { '/', '-', '\\', '|' };
  const int nbars = sizeof(bars) / sizeof(char);

  static int old_percent = -1;
  static int pos = 0;
  
  // Reset progress
  if (percent < 0) {
    old_percent = -1;
    pos = 0;
  }

  // Only print updated progress
  if (percent != old_percent) {
    printf("%c  %u%%\r", bars[pos], percent);
    fflush(stdout);
    pos = (pos + 1) % nbars;    
    old_percent = percent;
  }  
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_io::cmon_io(void)
{
  m_logfile = "";
  m_fd = -1;
  pthread_mutex_init(&m_write_mutex, NULL); // Use default mutex attributes
}

////////////////////////////////////////////////////////////////

void cmon_io::get_date_time_prefix(char *buffer,
				   unsigned len)
{
  time_t    now = time(NULL);
  struct tm *tstruct;

  // Get broken down time
  tstruct = localtime(&now);
  if ( tstruct == NULL ) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIME_ERROR,
	      "localtime failed", NULL);
  }

  // Current date/time, format is YYYY-MM-DD.HH:mm:ss
  if (strftime(buffer,
	       len,
	       "[%Y-%m-%d.%X] ",
	       tstruct) == 0) {
    
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_TIME_ERROR,
	      "strftime failed", NULL);
  }
}

////////////////////////////////////////////////////////////////

void cmon_io::write_all(int fd,
			const uint8_t *data,
			unsigned nbytes)
{
  unsigned total = 0;           // How many bytes written
  unsigned bytes_left = nbytes; // How many bytes left to write
  int n = 0;

  while (total < nbytes) {
    n = write(fd, data+total, bytes_left);
    if (n == -1) {
      THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
		"write failed, logfile (%s), bytes (%u) bytes left (%u)",
		m_logfile.c_str(), nbytes, bytes_left);
    }
    total += n;
    bytes_left -= n;
  }
}
