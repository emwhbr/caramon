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

#ifndef __BASIC_MEMORY_POOL_H__
#define __BASIC_MEMORY_POOL_H__

#include <pthread.h>

#include <stack>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Return codes
#define BMP_SUCCESS       0
#define BMP_MUTEX_ERROR  -1
#define BMP_MAX_LIMIT    -2
#define BMP_FAILURE      -3

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

template <class T> 
class basic_memory_pool {

 public:
  basic_memory_pool(void);                 // Create unlimited pool
  basic_memory_pool(unsigned nr_elements); // Create limited pool
  ~basic_memory_pool(void);

  long allocate(void **p); // Allocate memory for an instance of T 
                           // (remove memory from the pool)

  long deallocate(T *o);   // Deallocate an instance of T
                           // (add memory to the pool)

  long clear(void);        // Delete all of the memory in the pool

  long nr_elements(unsigned &value);

 private:
  bool m_max_pool_limit;

  std::stack<T*> m_free; // Stack to hold pointers to free chunks

  pthread_mutex_t m_pool_mutex;
};

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

template <class T>
basic_memory_pool<T>::basic_memory_pool(void)
{
  m_max_pool_limit = false;  // Unlimied pool
  pthread_mutex_init(&m_pool_mutex, NULL); // Use default mutex attributes
}

////////////////////////////////////////////////////////////////

template <class T>
basic_memory_pool<T>::basic_memory_pool(unsigned nr_elements)
{
  std::stack<T*> s;
  T *item_p;

  pthread_mutex_init(&m_pool_mutex, NULL); // Use default mutex attributes

  m_max_pool_limit = false;  // Start with unlimited pool

  // Create the pool (new elements)
  for (unsigned i=0; i < nr_elements; i++) {
    void *free_space = NULL;
    this->allocate(&free_space);
    item_p = new (free_space) T();
    s.push(item_p);
  }

  // Giva back all elements to the pool
  while (!s.empty()) {
    this->deallocate(s.top());
    s.pop();
  }

  if (nr_elements) {
    m_max_pool_limit = true; // Limited pool
  }
}

////////////////////////////////////////////////////////////////

template <class T>
basic_memory_pool<T>::~basic_memory_pool(void)
{
  this->clear();
  pthread_mutex_destroy(&m_pool_mutex);
}

////////////////////////////////////////////////////////////////

template <class T>
long basic_memory_pool<T>::allocate(void **p)
{
  void *place;

  // Lockdown the pool
  if (pthread_mutex_lock(&m_pool_mutex)) {
    return BMP_MUTEX_ERROR;
  }

  if (m_free.empty()) {
    if (m_max_pool_limit) {  // Limited pool
      *p = NULL;
      pthread_mutex_unlock(&m_pool_mutex); // Lockup the pool
      return BMP_MAX_LIMIT;
    }
    else {
      place = operator new(sizeof(T));    
    }
  }
  else {
    place = static_cast<void*>(m_free.top());
    m_free.pop();
  }

  *p = place;

  // Lockup the pool
  if (pthread_mutex_unlock(&m_pool_mutex)) {
    return BMP_MUTEX_ERROR;
  }

  return BMP_SUCCESS;
}

////////////////////////////////////////////////////////////////

template <class T>
long basic_memory_pool<T>::deallocate(T *o)
{
  // Lockdown the pool
  if (pthread_mutex_lock(&m_pool_mutex)) {
    return BMP_MUTEX_ERROR;
  }

  o->~T();
  m_free.push(o);

  // Lockup the pool
  if (pthread_mutex_unlock(&m_pool_mutex)) {
    return BMP_MUTEX_ERROR;
  }

  return BMP_SUCCESS;
}

////////////////////////////////////////////////////////////////

template <class T>
long basic_memory_pool<T>::clear(void)
{
  // Lockdown the pool
  if (pthread_mutex_lock(&m_pool_mutex)) {
    return BMP_MUTEX_ERROR;
  }

  // Delete all of the available memory in pool
  while (!m_free.empty()) {
    ::operator delete(m_free.top());
    m_free.pop();
  }

  // Lockup the pool
  if (pthread_mutex_unlock(&m_pool_mutex)) {
    return BMP_MUTEX_ERROR;
  }

  return BMP_SUCCESS;
}

////////////////////////////////////////////////////////////////

template <class T>
long basic_memory_pool<T>::nr_elements(unsigned &value)
{
  // Lockdown the pool
  if (pthread_mutex_lock(&m_pool_mutex)) {
    return BMP_MUTEX_ERROR;
  }

  value =  m_free.size();

  // Lockup the pool
  if (pthread_mutex_unlock(&m_pool_mutex)) {
    return BMP_MUTEX_ERROR;
  }

  return BMP_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

#endif // __BASIC_MEMORY_POOL_H__
