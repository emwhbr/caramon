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

#ifndef __CMON_LED_H__
#define __CMON_LED_H__

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of types
/////////////////////////////////////////////////////////////////////////////
// Available LEDs
typedef enum {
  CMON_LED_ALIVE,
  CMON_LED_SYSFAIL,
  CMON_LED_MAX_LEDS
} CMON_LED;

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////

extern void cmon_led_initialize(void);
extern void cmon_led_finalize(void);

extern void cmon_led(CMON_LED led,
		     bool activate);

#endif // __CMON_LED_H__
