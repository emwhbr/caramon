// ************************************************************************
// *                                                                      *
// * Copyright (C) 2016 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#ifndef __CMON_ALIVE_H__
#define __CMON_ALIVE_H__

#include "thread.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_alive : public thread {
 public:
  cmon_alive(string thread_name,
	     uint32_t cpu_affinity_mask,
	     int rt_priority,
	     bool verbose);

  ~cmon_alive(void);

  void shutdown(void);

 protected:
  virtual long setup(void);         // Implements pure virtual functions from base class
  virtual long cleanup(void);       // Implements pure virtual functions from base class
  virtual long execute(void *arg);  // Implements pure virtual functions from base class

 private:
  bool m_verbose;

  // Controlled shutdown
  bool m_shutdown_requested;

  void initialize_alive(void);
  void finalize_alive(void);
  void handle_alive(void);

  // Support functions for the internal thread
};

#endif // __CMON_ALIVE_H__
