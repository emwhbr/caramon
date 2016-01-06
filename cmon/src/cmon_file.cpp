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

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include "cmon_file.h"
#include "cmon_exception.h"
#include "cmon.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cmon_file::cmon_file(const string pathname)
{
  m_pathname = pathname;
  m_fd = -1;
}

////////////////////////////////////////////////////////////////

cmon_file::~cmon_file(void)
{
}

////////////////////////////////////////////////////////////////

unsigned cmon_file::free_disk_space_size(void)
{
  struct statvfs fs_info;

  char *path = strdup(m_pathname.c_str());

  if (statvfs(dirname(path), &fs_info) == -1) {
    THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
	      "statvfs failed, file(%s)",
	      m_pathname.c_str());
  }

  return (fs_info.f_bsize * fs_info.f_bavail); // Free blocks -> bytes
}

////////////////////////////////////////////////////////////////

bool cmon_file::exists(void)
{
  int rc;

  // Check if file exists
  rc = access(m_pathname.c_str(), F_OK);
  if (rc == 0) {
    return true;
  }
  else {
    if (errno == ENOENT) {
      return false;
    }
    else {
      THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
		"access failed, file(%s)",
		m_pathname.c_str());
    }
  }
}

////////////////////////////////////////////////////////////////

void cmon_file::open_file(unsigned flags)
{
  int rc;

  const int file_mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  int file_flags;

  // Check flags
  if ( (flags & CMON_FILE_WRITEONLY) &&
       (flags & CMON_FILE_READONLY) ) {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_BAD_ARGUMENT,
	      "open failed, file(%s), bad combination of flags = 0x%u",
	      m_pathname.c_str(), flags);
  }

  // Set open flags
  if (flags & CMON_FILE_WRITEONLY) {
    file_flags = (O_WRONLY | O_CREAT);
  }
  else if (flags & CMON_FILE_READONLY) {
    file_flags = (O_RDONLY);
  }
  else {
    THROW_EXP(CMON_INTERNAL_ERROR, CMON_BAD_ARGUMENT,
	      "open failed, file(%s), unsupported flags = 0x%u",
	       m_pathname.c_str(), flags);
  }

  // Open file
  rc = open(m_pathname.c_str(), file_flags, file_mode);
  if (rc == -1) {
    THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
	      "open failed, file(%s)",
	      m_pathname.c_str());
  }

  m_fd = rc;
}

////////////////////////////////////////////////////////////////

void cmon_file::write_file(const uint8_t *data,
			   unsigned nbytes)
{
  // NOTE!
  // Assumes file already open.

  unsigned total = 0;
  unsigned bytes_left = nbytes;

  int n = 0;

  while (total < nbytes) {
    n = write(m_fd, data+total, bytes_left);
    if (n == -1) {
      THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
		"write failed, file(%s), nbytes(%u), bytes left(%u)",
		m_pathname.c_str(), nbytes, bytes_left);
    }
    total += n;
    bytes_left -= n;
  }
}

////////////////////////////////////////////////////////////////

void cmon_file::read_file(uint8_t *data,
			  unsigned nbytes,
			  bool all)
{
  // NOTE!
  // Assumes file already open.

  unsigned total = 0;
  unsigned bytes_left = nbytes;

  int n = 0;

  while (total < nbytes) {
    n = read(m_fd, data+total, bytes_left);
    if ( (n == -1) || ((n == 0) && all) ) {
      THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
		"read failed, file(%s), nbytes(%u), bytes left(%u)",
		m_pathname.c_str(), nbytes, bytes_left);
    }
    else if (n == 0) {
      break; // EOF before nbytes was read, but caller doesn't care
    }
    total += n;
    bytes_left -= n;
  }
}

////////////////////////////////////////////////////////////////

bool cmon_file::is_eof(void)
{
  // NOTE!
  // Assumes file already open.

  struct stat f_info;
  off_t current_offset;

  // Get file info
   if (fstat(m_fd, &f_info) == -1) {
    THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
	      "is_eof failed, fstat for file(%s)",
	      m_pathname.c_str());
  }
  
  // Check current position
  current_offset = lseek(m_fd, 0, SEEK_CUR);
  if (current_offset == -1) {
    THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
	      "is_eof failed, lseek for file(%s)",
	      m_pathname.c_str());
  }

  // Check EOF
  if (f_info.st_size == current_offset) {
    return true;
  }
  else {
    return false;
  }
}

////////////////////////////////////////////////////////////////

void cmon_file::close_file(void)
{
  if ( fsync(m_fd) == -1 ) {
    THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
	      "fsync failed, file(%s)",
	      m_pathname.c_str());
  }

  if ( close(m_fd) == -1 ) {
    THROW_EXP(CMON_LINUX_ERROR, CMON_FILE_OPERATION_FAILED,
	      "close failed, file(%s)",
	      m_pathname.c_str());
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

