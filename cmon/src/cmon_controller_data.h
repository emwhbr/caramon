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

#ifndef __CMON_CONTROLLER_DATA_H__
#define __CMON_CONTROLLER_DATA_H__

#include <time.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
typedef struct {
  struct tm tstruct;
} CMON_CONTROLLER_TIME;

typedef struct {
  bool  valid;
  float min;
  float max;
  float mean;
} CMON_CONTROLLER_ITEM;

typedef struct {
  CMON_CONTROLLER_TIME time;
  CMON_CONTROLLER_ITEM duty;
  CMON_CONTROLLER_ITEM set_value;
} CMON_CONTROLLER_DATA;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_controller_data {
 public:
  cmon_controller_data(void);
  virtual ~cmon_controller_data(void);
  
  CMON_CONTROLLER_DATA m_controller_data;
};

#endif // __CMON_CONTROLLER_DATA_H__
