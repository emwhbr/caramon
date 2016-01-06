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
#include <stdlib.h>
#include <stdarg.h>
#include <execinfo.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sstream>
#include <iomanip>

#include "cmon_exception.h"
#include "cmon_utility.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////
static string get_class_method(const string pretty_function);

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_exception::cmon_exception(const char *file,
			       int line,
			       const char *pretty_function,
			       CMON_ERROR_SOURCE source,
			       long code,
			       const char *info_format, ...)
{	
  // Get list of void pointers, return addresses for each stack frame
  bzero(m_stack_frames, sizeof(void *) * MAX_NR_STACK_FRAMES);
  m_nr_frames = backtrace(m_stack_frames, MAX_NR_STACK_FRAMES);

  // Handle the standard predefined macros
  m_file = file;
  m_line = line;
  m_pretty_function = pretty_function;

  // Handle internal error info
  m_source = source;
  m_code   = code;
	
  // Retrieve any additional arguments for the format string
  char info_buffer[512];
  va_list info_args;
  va_start(info_args, info_format);
  vsprintf(info_buffer, info_format, info_args);
  va_end(info_args);

  m_info = info_buffer;

  // Retreive identification
  m_thread_id  = cmon_get_my_thread_id();
  m_process_id = cmon_get_my_pid();
}

////////////////////////////////////////////////////////////////

cmon_exception::~cmon_exception(void) throw()
{
}

////////////////////////////////////////////////////////////////

string cmon_exception::get_function(void)
{
  return get_class_method(m_pretty_function);
}

////////////////////////////////////////////////////////////////

void cmon_exception::get_stack_frames(CMON_STACK_FRAMES &frames)
{
  bzero(&frames, sizeof(frames));

  for (int i=0; i < m_nr_frames; i++) {
    frames.frames[i] = (uint32_t)m_stack_frames[i];
  }

  frames.active_frames = m_nr_frames;
}

////////////////////////////////////////////////////////////////

string cmon_exception::get_details()
{
  // Get the stack trace
  ostringstream oss_msg;
  char buffer[18];

  oss_msg << "stack frames:" << (int)m_nr_frames << "\n";

  for (int i=0; i < m_nr_frames; i++) {
    sprintf(buffer, "0x%08x", (uint32_t)m_stack_frames[i]);
    oss_msg << "\tframe:" << dec << setw(2) << setfill('0') << i
	    << "  addr:" << buffer << "\n";
  }

  // Get identification
  oss_msg << "Pid: " << m_process_id 
	  << ", Tid: " << m_thread_id << "\n";

  // Get info from predefined macros
  oss_msg << "Violator: " << m_file 
	  << ":" << m_line
	  << ", " << get_class_method(m_pretty_function) << "\n";

  // Get the internal info
  oss_msg << "Source: " << m_source
	  << ", Code: " << m_code << "\n";

  oss_msg << "Info: " << m_info << "\n";

  // Source of error
  switch (m_source) {
  case CMON_INTERNAL_ERROR:
    oss_msg << "CMON_INTERNAL_ERROR";
    break;
  case CMON_LINUX_ERROR:
    oss_msg << "CMON_LINUX_ERROR - errno:" 
	    << errno << " => " << strerror(errno);
    break;
  }
	
  return oss_msg.str();
}

////////////////////////////////////////////////////////////////

string cmon_exception::get_details_syslog()
{
	// Note!!
  // To avoid problems with syslog multiline-messages (embedded '\n')
  // we use '\\n' as a newline separator.
  // This message can be decoded as a multiline message by examine
  // the error log using sed-command:
  // tail -f /var/log/errors.log | sed 's/\\n/\n/g'
	
  // Get the stack trace
  ostringstream oss_msg;
  char buffer[18];

  oss_msg << "\\n";
  oss_msg << "stack frames:" << (int)m_nr_frames << "\\n";

  for (int i=0; i < m_nr_frames; i++) {
    sprintf(buffer, "0x%08x", (uint32_t)m_stack_frames[i]);
    oss_msg << "frame:" << dec << setw(2) << setfill('0') << i
	    << "  addr:" << buffer << "\\n";
  }

  // Get identification
  oss_msg << "Pid: " << m_process_id 
	  << ", Tid: " << m_thread_id << "\\n";

  // Get info from predefined macros
  oss_msg << "Violator: " << m_file 
	  << ":" << m_line
	  << ", " << get_class_method(m_pretty_function) << "\\n";

  // Get the internal info
  oss_msg << "Source: " << m_source
	  << ", Code: " << m_code << "\\n";

  oss_msg << "Info: " << m_info << "\\n";

  // Source of error
  switch (m_source) {
  case CMON_INTERNAL_ERROR:
    oss_msg << "CMON_INTERNAL_ERROR";
    break;
  case CMON_LINUX_ERROR:
    oss_msg << "CMON_LINUX_ERROR - errno:" 
	    << errno << " => " << strerror(errno);
    break;
  }

  return oss_msg.str();
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               File private support functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

static string get_class_method(const string pretty_function)
{
  string class_method = pretty_function;

  // Strip the parameter list
  size_t index = class_method.find("(");
  if (index == string::npos) {
    return class_method;  // Degenerated case 
  }
  class_method.erase( index );

  // Strip the return type
  index = class_method.rfind(" ");
  if (index == string::npos) {
    return class_method;  // Degenerated case
  }
  class_method.erase(0, index + 1);

  return class_method; // The stripped name = class::method
}
