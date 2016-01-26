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

#include "pid_ctrl.h"

// Implementation notes:
// 1. PID without a PhD, Tim Wescott
//    http://m.eet.com/media/1112634/f-wescot.pdf
//
// 2. Arduino PID library
//    http://playground.arduino.cc/Code/PIDLibrary
//

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

pid_ctrl::pid_ctrl(double command_position,
		   double p_gain,
		   double i_gain,
		   double d_gain,
		   PID_CTRL_DIRECTION direction)
{
  this->set_command_position(command_position);
  this->set_output_limits(0.0, 100.0);

  m_output = m_output_min;

  m_p_gain = p_gain;

  m_i_gain = i_gain;
  m_i_state = 0.0;

  m_d_gain = d_gain;
  m_d_state = command_position;

  if (direction == PID_CTRL_REVERSE) {
    m_p_gain = -m_p_gain;
    m_i_gain = -m_i_gain;
    m_d_gain = -m_d_gain;
  }
}

////////////////////////////////////////////////////////////////

pid_ctrl::~pid_ctrl(void)
{
}

////////////////////////////////////////////////////////////////

void pid_ctrl::set_output_limits(double min,
				 double max)
{
  m_output_min = min;
  m_output_max = max;

  m_i_min = m_output_min;
  m_i_max = m_output_max;
}

////////////////////////////////////////////////////////////////

void pid_ctrl::set_command_position(double value)
{
  m_command_position = value;
}

////////////////////////////////////////////////////////////////

double pid_ctrl::get_command_position(void)
{
  return  m_command_position;
}

////////////////////////////////////////////////////////////////

double pid_ctrl::update(double position)
{
  double pos_error;

  double p_term;
  double i_term;
  double d_term;

  double output;

  // Calculate error
  pos_error = m_command_position - position;

  // Calculate PROPORTIONAL term
  p_term = m_p_gain * pos_error;

  // Calculate INTEGRAL term
  m_i_state += pos_error;
  if (m_i_state > m_i_max) {
    m_i_state = m_i_max;
  }
  else if (m_i_state < m_i_min) {
    m_i_state = m_i_min;
  }
  i_term = m_i_gain * m_i_state;

  // Calculate DERIVATE term
  d_term = m_d_gain * (m_d_state - position);
  m_d_state = position;

  // Limit output
  output =  p_term + d_term + i_term;
  if (output > m_output_max) {
    output = m_output_max;
  }
  else if (output < m_output_min) {
    output = m_output_min;
  }
  m_output = output;
  return output;
}

////////////////////////////////////////////////////////////////

double pid_ctrl::get_output(void)
{
  return  m_output;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////




