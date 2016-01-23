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

#ifndef __PID_CTRL_H__
#define __PID_CTRL_H__

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
typedef enum {
  PID_CTRL_DIRECT,
  PID_CTRL_REVERSE
} PID_CTRL_DIRECTION; // The PID will connected to a:
                      // - DIRECT acting process  (+Output --> +Input)
                      // - REVERSE acting process (+Output --> -Input)

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class pid_ctrl {
 public:
  pid_ctrl(double command_position,
	   double p_gain,
	   double i_gain,
	   double d_gain,
	   PID_CTRL_DIRECTION direction);

  ~pid_ctrl(void);

  void set_output_limits(double min,
			 double max); // Limits the output to a specific range, default 0-100.
				      // The user will want to change this depending on
				      // the application.

  void set_command_position(double value); // Update set-point.

  double update(double position); // Performs the PID calculation.

 private:
  double m_command_position;
  double m_output_min;
  double m_output_max;

  // Proportional
  double m_p_gain; // (P)roportional gain

  // Integral
  double m_i_gain;  // (I)ntegral gain
  double m_i_state; // Integrator state
  double m_i_min;   // Minimum allowable integrator state
  double m_i_max;   // Maximum allowable integrator state
  
  // Derivate
  double m_d_gain;  // (D)erivative gain
  double m_d_state; // Last position input
};

#endif // __PID_CTRL_H__
