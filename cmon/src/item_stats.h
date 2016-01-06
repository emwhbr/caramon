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

#ifndef __ITEM_STATS_H__
#define __ITEM_STATS_H__

#include <vector>
#include <numeric>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Exceptions
const int item_stats_no_elements = 1; // Raised when no elements available

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
template <class T> 
class item_stats {
 public:
  item_stats(void);
  ~item_stats(void);

  void insert(const T &item);

  void reset(void);

  T get_min(void);
  T get_max(void);
  double get_mean(void);

 private:
  vector<T> m_item_list;

  void check_available_elements(void);
};

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

template <class T>
item_stats<T>::item_stats(void)
{
}

////////////////////////////////////////////////////////////////

template <class T>
item_stats<T>::~item_stats(void)
{
}

////////////////////////////////////////////////////////////////

template <class T>
void item_stats<T>::insert(const T &item)
{  
  m_item_list.push_back(item);
}

////////////////////////////////////////////////////////////////

template <class T>
void item_stats<T>::reset(void)
{
  m_item_list.clear();
}

////////////////////////////////////////////////////////////////

template <class T>
T item_stats<T>::get_min(void)
{
  this->check_available_elements();

  T min_item = m_item_list[0];
  for (unsigned i=0; i < m_item_list.size(); i++) {
    if (m_item_list[i] < min_item) {
      min_item = m_item_list[i];
    }
  }
  return min_item;
}

////////////////////////////////////////////////////////////////

template <class T>
T item_stats<T>::get_max(void)
{
  this->check_available_elements();

  T max_item = m_item_list[0];
  for (unsigned i=0; i < m_item_list.size(); i++) {
    if (m_item_list[i] > max_item) {
      max_item = m_item_list[i];
    }
  }
  return max_item;
}

////////////////////////////////////////////////////////////////

template <class T>
double item_stats<T>::get_mean(void)
{
  this->check_available_elements();

  double sum = accumulate(m_item_list.begin(),
			  m_item_list.end(),
			  (double)0.0);

  return sum / (double)m_item_list.size();
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

template <class T>
void item_stats<T>::check_available_elements(void)
{
  if (!m_item_list.size()) {
    throw item_stats_no_elements;
  }
}

#endif // __ITEM_STATS_H__
