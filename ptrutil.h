//
//  Copyright (c) 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef ptrutil_h
#define ptrutil_h

#include <memory>


namespace util {

// Some smart pointers for dealing with Xlib and the like.
template <class T>
using x_unique_ptr = std::unique_ptr<T, int(*)(void *)>;

template <class T>
using x2_unique_ptr = std::unique_ptr<T, void(*)(T*)>;

} // namespace util

#endif
