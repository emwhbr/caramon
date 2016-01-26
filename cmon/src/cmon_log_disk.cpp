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

#include "cmon_log_disk.h"
#include "cmon_io.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_log_disk::
cmon_log_disk(string name,
	      bool verbose,
	      string climate_data_file_name) : cmon_log(name,
							verbose)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_log_disk::cmon_log_disk\n", m_name.c_str());
  }
  m_climate_data_file = new cmon_file(climate_data_file_name);
}

////////////////////////////////////////////////////////////////

cmon_log_disk::~cmon_log_disk(void)
{
  if (m_verbose) {
    cmon_io_put("%s : cmon_log_disk::~cmon_log_disk\n", m_name.c_str());
  }
  delete m_climate_data_file;
}

////////////////////////////////////////////////////////////////

void cmon_log_disk::initialize(void)
{
  m_climate_data_file->open_file(CMON_FILE_WRITEONLY);
}

////////////////////////////////////////////////////////////////

void cmon_log_disk::finalize(void)
{
  m_climate_data_file->close_file();
}

////////////////////////////////////////////////////////////////

void cmon_log_disk::log_data(const CMON_CLIMATE_DATA *climate_data,
			     const CMON_CONTROLLER_DATA *controller_data)
{
  // NOTE!
  // Creates an entry in climate data file according to:
  // <date&time>;<internal mean temp>;<internal mean hum>;<external mean temp>;<pid duty>;<pid set value>

  // Decorate message with date and time (from climate data)
  ostringstream ossMsg;
  ossMsg.str("");
  ossMsg << this->get_iso8601_date_time_str(&climate_data->time.tstruct) << ";";

  // Internal climate
  if (climate_data->internal_temperature.valid) {
    ossMsg << fixed << setw(8) << setprecision(3)
	   << climate_data->internal_temperature.mean << ";";
  }
  else {
    ossMsg << "not valid" << ";";
  }
  if (climate_data->internal_humidity.valid) {    
    ossMsg << fixed << setw(8) << setprecision(3)
	   << climate_data->internal_humidity.mean << ";";
  }
  else {
    ossMsg << "not valid" << ";";
  }

  // External climate
  if (climate_data->external_temperature.valid) {
    ossMsg << fixed << setw(8) << setprecision(3)
	   << climate_data->external_temperature.mean << ";";
  }
  else {
    ossMsg << "not valid" << ";";
  }

  // Controller PID
  if (controller_data->temp_controller.valid) {
    ossMsg << fixed << setw(8) << setprecision(2)
	   << controller_data->temp_controller.duty << ";";
    ossMsg << fixed << setw(8) << setprecision(3)
	   << controller_data->temp_controller.set_value << ";";
  }
  else {
    ossMsg << "not valid" << ";";
    ossMsg << "not valid" << ";";
  }

  ossMsg << "\n";

  // Log to file
  m_climate_data_file->write_file((const uint8_t *)ossMsg.str().c_str(),
				  (unsigned)ossMsg.str().length());
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

