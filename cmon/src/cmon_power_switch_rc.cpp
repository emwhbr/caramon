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

#include <sstream>

#include "cmon_power_switch_rc.h"
#include "cmon_exception.h"
#include "cmon_io.h"
#include "shell_cmd.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define TX433_CMD          "/caramon/tx433"
#define TX433_CMD_EXIT_OK  0

#define TX433_ON   "1"
#define TX433_OFF  "0"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_power_switch_rc::
cmon_power_switch_rc(string name,
		     uint8_t system_code,
		     uint8_t unit_code,
		     bool verbose) : cmon_power_switch(name, verbose) 
{
  m_system_code = system_code;
  m_unit_code = unit_code;
}

////////////////////////////////////////////////////////////////

cmon_power_switch_rc::~cmon_power_switch_rc(void)
{
}

////////////////////////////////////////////////////////////////

void cmon_power_switch_rc::switch_on(void)
{
  ostringstream ossMsg;

  ossMsg << string(TX433_CMD) << " "
	 << (unsigned)m_system_code << " "
	 << (unsigned)m_unit_code << " "
	 << string(TX433_ON);

  this->execute_tx433_cmd(ossMsg.str());
}

////////////////////////////////////////////////////////////////

void cmon_power_switch_rc::switch_off(void)
{
  ostringstream ossMsg;

  ossMsg << string(TX433_CMD) << " "
	 << (unsigned)m_system_code << " "
	 << (unsigned)m_unit_code << " "
	 << string(TX433_OFF);

  this->execute_tx433_cmd(ossMsg.str());
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_power_switch_rc::execute_tx433_cmd(const string cmd)
{

  shell_cmd sc_tx433;
  int exit_status;

  if (m_verbose) {
    cmon_io_put("%s : %s\n",
		m_name.c_str(), cmd.c_str());
  }

  // Execute command
  if (sc_tx433.execute(cmd, exit_status) != SHELL_CMD_SUCCESS) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_SHELL_OPERATION_FAILED,
	      "%s : failed to execute command : %s",
	      m_name.c_str(), cmd.c_str());
  }

  // Check command exit status
  if (exit_status != TX433_CMD_EXIT_OK) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_SHELL_OPERATION_FAILED,
	      "%s : command failed : %s, exit status : %d",
	      m_name.c_str(), cmd.c_str(), exit_status);
  }
}
