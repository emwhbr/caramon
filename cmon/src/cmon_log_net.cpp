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

#include <sstream>
#include <iomanip>

#include "cmon_log_net.h"
#include "cmon_exception.h"
#include "cmon_io.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define CURL_CMD          "/usr/bin/curl"
#define CURL_CMD_EXIT_OK  0

#define THINGSPEAK_URL            "https://api.thingspeak.com/update"
#define THINGSPEAK_WRITE_API_KEY  "G7DGBX4MO7JF89P1" // Channel 75139, caramonpi

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_log_net::
cmon_log_net(string name,
	     bool verbose) : cmon_log(name,
				      verbose)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_log_net::cmon_log_net\n", m_name.c_str());
  }
}

////////////////////////////////////////////////////////////////

cmon_log_net::~cmon_log_net(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_log_net::~cmon_log_net\n", m_name.c_str());
  }
}

////////////////////////////////////////////////////////////////

void cmon_log_net::initialize(void)
{
}

////////////////////////////////////////////////////////////////

void cmon_log_net::finalize(void)
{
}

////////////////////////////////////////////////////////////////

void cmon_log_net::log_data(const CMON_CLIMATE_DATA *climate_data,
			    const CMON_CONTROLLER_DATA *controller_data)
{
  // NOTE!
  // Sends data to Thingspeak channel using curl.
  // Channel is updated with the following command syntax:
  /*
   * curl --fail --silent \
   *      --data "key=<WRITE_KEY>&field1=<value1>&field2=<value2>&field3=<value3>&field4=<value4>&field5=<value5>&field6=<value6>" \
   *      https://api.thingspeak.com/update
  */

  ostringstream ossMsg;

  string curl_cmd = string(CURL_CMD) + " --fail --silent --data";
  curl_cmd += (" \"key=" + string(THINGSPEAK_WRITE_API_KEY));

  // Internal temperature (Field 1)
  if (climate_data->internal_temperature.valid) {
    ossMsg.str("");
    ossMsg << fixed << setprecision(3) << climate_data->internal_temperature.mean;
    curl_cmd += ("&field1=" + ossMsg.str());
  }

  // Internal humidity (Field 2)
  if (climate_data->internal_humidity.valid) {
    ossMsg.str("");
    ossMsg << fixed << setprecision(3) << climate_data->internal_humidity.mean;
    curl_cmd += ("&field2=" + ossMsg.str());
  }

  // External temperature 1 (Field 3)
  if (climate_data->external_temperature_1.valid) {
    ossMsg.str("");
    ossMsg << fixed << setprecision(3) << climate_data->external_temperature_1.mean;
    curl_cmd += ("&field3=" + ossMsg.str());
  }

  // External temperature 2 (Field 4)
  if (climate_data->external_temperature_2.valid) {
    ossMsg.str("");
    ossMsg << fixed << setprecision(3) << climate_data->external_temperature_2.mean;
    curl_cmd += ("&field4=" + ossMsg.str());
  }

  // Controller PID - duty (Field 5)
  if (controller_data->duty.valid) {
    ossMsg.str("");
    ossMsg << fixed << setprecision(2) << controller_data->duty.mean;
    curl_cmd += ("&field5=" + ossMsg.str());
  }

  // Controller PID - set value (Field 6)
  if (controller_data->set_value.valid) {
    ossMsg.str("");
    ossMsg << fixed << setprecision(3) << controller_data->set_value.mean;
    curl_cmd += ("&field6=" + ossMsg.str());
  }

  curl_cmd += "\" ";
  curl_cmd += string(THINGSPEAK_URL);

  // Write data to Thingspeak channel
  this->execute_curl_cmd(curl_cmd);
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void cmon_log_net::execute_curl_cmd(const string cmd)
{
  int exit_status;

  // Execute command
  if (m_sc.execute(cmd, exit_status) != SHELL_CMD_SUCCESS) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_SHELL_OPERATION_FAILED,
	      "Failed to execute command : %s", cmd.c_str());
  }

  // Check command exit status
  if (exit_status != CURL_CMD_EXIT_OK) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_SHELL_OPERATION_FAILED,
	      "Command failed : %s, exit status : %d",
	      cmd.c_str(), exit_status);
  }
}
