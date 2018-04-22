//
//  Copyright (c) 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef scache_h
#define scache_h

#include <cstdint>
#include <utility>



// "Fancy" wrapper around a global var to invalidate the cached stat data.
class LoopCounter {
public:
    static void increment(void) { _counter++; }
    static uint64_t count(void) { return _counter; }

private:
    static uint64_t _counter;
};



// Data is cached (and valid) until the LoopCounter increments.
template <class X>
class StatCache : public LoopCounter {
public:
    StatCache(void) : _validCount(0) {}
    StatCache(const X &x) : _cache(x), _validCount(count()) {}

    void set(const X &x) {
        _cache = x;
        validate();
    }

    void set(X &&x) {
        _cache = std::move(x);
        validate();
    }

    const X &get(void) const { return _cache; }

    bool valid(void) const { return count() == _validCount; }
    void validate(void) { _validCount = count(); }

private:
    X _cache;
    uint64_t _validCount;
};

#endif
