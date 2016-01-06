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

#ifndef __CMON_IO_H__
#define __CMON_IO_H__

#include <pthread.h>
#include <string>
#include <stdint.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define cmon_io_initialize   cmon_io::instance()->initialize
#define cmon_io_finalize     cmon_io::instance()->finalize
#define cmon_io_put          cmon_io::instance()->put
#define cmon_io_progress_bar cmon_io::instance()->progress_bar

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class cmon_io {
 public:
  ~cmon_io(void);
  static cmon_io* instance(void);
  
  void initialize(string logfile);
  void finalize(void);
  
  void put(const char *format, ...);
  void put(string message);

  void progress_bar(int percent);
  
 private:
  static cmon_io *m_instance;
  string          m_logfile;
  int             m_fd;
  pthread_mutex_t m_write_mutex;

  cmon_io(void); // Private constructor
                 // so it can't be called

  void get_date_time_prefix(char *buffer,
                            unsigned len);
  
  void write_all(int fd,
              	 const uint8_t *data,
                 unsigned nbytes);
};

#endif // __CMON_IO_H__
