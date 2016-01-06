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

#ifndef __CMON_FILE_H__
#define __CMON_FILE_H__

#include <stdint.h>

#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
// Flag values
#define CMON_FILE_READONLY   0x01
#define CMON_FILE_WRITEONLY  0x02

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class cmon_file {
 public:
  cmon_file(const string pathname);
  ~cmon_file(void);

  unsigned free_disk_space_size(void);

  bool exists(void);

  void open_file(unsigned flags);

  void write_file(const uint8_t *data,
		  unsigned nbytes);

  void read_file(uint8_t *data,
		 unsigned nbytes,
		 bool all);

  bool is_eof(void);

  void close_file(void);

 private:
  string m_pathname;
  int    m_fd;
};

#endif // __CMON_FILE_H__
