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

#ifndef __CMON_FALLBACK_H__
#define __CMON_FALLBACK_H__

#include "thread.h"
#include "cmon_power_switch.h"

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
class cmon_fallback : public thread {
 public:
  cmon_fallback(string thread_name,
		uint32_t cpu_affinity_mask,
		int rt_priority,
		bool verbose);

  ~cmon_fallback(void);

  void shutdown(void);

 protected:
  virtual long setup(void);         // Implements pure virtual functions from base class
  virtual long cleanup(void);       // Implements pure virtual functions from base class
  virtual long execute(void *arg);  // Implements pure virtual functions from base class

 private:
  bool m_verbose;
  bool m_fallback_led_active;

  // Controlled shutdown
  bool m_shutdown_requested;
  
  // Contols wall outlet (230V) for radiator
  cmon_power_switch *m_radiator_switch;

  void initialize_fallback(void);
  void finalize_fallback(void);
  void handle_fallback(void);

  // Support functions for the internal thread
  void radiator_pulse(bool on,
		      double pulse_time_sec);

  void radiator_on(void);

  void radiator_off(void);
};

#endif // __CMON_FALLBACK_H__
