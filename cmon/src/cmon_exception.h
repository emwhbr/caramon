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

#ifndef __CMON_EXCEPTION_H__
#define __CMON_EXCEPTION_H__

#include <stdint.h>
#include <string>

#include "cmon.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define MAX_NR_STACK_FRAMES  32

#define EXP(source, code, info_format, ...) \
  cmon_exception(__FILE__, __LINE__, __PRETTY_FUNCTION__, \
		 source, code, info_format, ##__VA_ARGS__)

#define THROW_EXP(source, code, info_format, ...) \
  throw EXP(source, code, info_format, ##__VA_ARGS__)

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
typedef struct {
  uint8_t  active_frames;
  uint32_t frames[MAX_NR_STACK_FRAMES];
} CMON_STACK_FRAMES;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class cmon_exception : public exception {

 public:
  cmon_exception(const char *file,
		 int line,
		 const char *pretty_function,
		 CMON_ERROR_SOURCE source,
		 long code,
		 const char *info_format, ...);
  ~cmon_exception(void) throw();

  string get_file(void) {return m_file;}
  int get_line(void)    {return m_line;}
  string get_function(void);

  CMON_ERROR_SOURCE get_source(void) {return m_source;}
  long get_code(void)                {return m_code;}
  string get_info(void)              {return m_info;}

  long get_thread_id(void)  {return m_thread_id;}
  long get_process_id(void) {return m_process_id;}

  void get_stack_frames(CMON_STACK_FRAMES &frames);
  
  string get_details(void);
  string get_details_syslog(void);

 private:
  string m_file;
  int    m_line;
  string m_pretty_function;

  CMON_ERROR_SOURCE m_source;
  long              m_code;
  string            m_info;

  long m_thread_id;
  long m_process_id;

  int  m_nr_frames;
  void *m_stack_frames[MAX_NR_STACK_FRAMES];
};

#endif // __CMON_EXCEPTION_H__
