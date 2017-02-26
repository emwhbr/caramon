// ************************************************************************
// *                                                                      *
// * Copyright (C) 2017 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#ifndef __CMON_WDT_H__
#define __CMON_WDT_H__

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////

extern void cmon_wdt_initialize(void);
extern void cmon_wdt_finalize(void);

extern void cmon_wdt_enable(void);
extern void cmon_wdt_disable(void);

extern void cmon_wdt_keep_alive(void);

#endif // __CMON_WDT_H__
