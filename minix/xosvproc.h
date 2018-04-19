//
//  Copyright (c) 2015, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef xosvproc_h
#define xosvproc_h


#include <string>
#include <vector>

#include <cstdint>  // for uint64_t for 32 bit systems


//---------------------------------------------------------------
// This is loosly based around the 'struct proc' from the minix
// kernel header proc.h and is loaded from the /proc filesystem
// based on the code from 3.4.0/minix/fs/procfs/pid.c.  It was
// the only documentation I found on this subject.  The comments
// after each field are straight out of pid.c.
//---------------------------------------------------------------
struct XOSVProc {
    pid_t pid;

    //------ spit out by proffs/pid.c --------------------------
    int psiVers;		// information version
    char ptype;			// process type
    int pend;			// process endpoint
    std::string name;		// process name
    char state;			// process state letter
    int blockedon;		// endpt blocked on, or NONE
    int ppri;			// process priority
    unsigned int usrtime;	// user time
    unsigned int systime;	// system time
    uint64_t execycles;		// execution cycles
    uint64_t kerncycles;	// kernel IPC cycles
    uint64_t kcallcycles;	// kernel call cycles
    unsigned long totmem;	// total memory
    int nice;			// nice value
    unsigned int euid;		// effective user ID
    //---------------------------------------------------------------

    std::istream &load(std::istream &is);
    std::ostream &dump(std::ostream &os) const;

    static std::vector<XOSVProc> ptable(void);
};


inline std::istream &operator>>(std::istream &is, XOSVProc &p) {
    return p.load(is);
}


inline std::ostream &operator<<(std::ostream &os, const XOSVProc &p) {
    return p.dump(os);
}


#endif
