//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef XOSVPROC_H
#define XOSVPROC_H


#include <string>
#include <vector>

#include <stdint.h>  // for uint64_t for 32 bit systems


//---------------------------------------------------------------
// This is loosly based around the 'struct proc' from the minix
// kernel header proc.h and is loaded from the /proc filesystem
// based on the code from proc.h and the minix top.c.  It was
// the only documentation I found on this subject.  The comments
// after each field are straight out of top.c and the type is
// based on top.c and procfs.c
//---------------------------------------------------------------
struct XOSVProc {
    pid_t pid;
    // ALL -------------
    int psiVers;              // PSINFO_VERSION,  /* information version */
    char ptype;               // type,            /* process type */
    int pend;                 // (int)proc[i].p_endpoint,/* process endpoint */
    std::string name;         // name,     /* process name */
    char state;               // state,   /* process state letter */
    int blockedon; // (int)P_BLOCKEDON(&proc[i]),/* endpt blocked on, or NONE */
    int ppri;                 // (int)proc[i].p_priority,/* process priority */
    unsigned long usrTime;    // (long)proc[i].p_user_time,/* user time */
    unsigned long sysTime;    // (long)proc[i].p_sys_time,/* system time */
    unsigned long cycles_hi;  // ex64hi(proc[i].p_cycles),/* execution cycles */
    unsigned long cycles_lo; // ex64lo(proc[i].p_cycles)
    uint64_t cycles;
    // END ALL -------------

    // If not TYPE_TASK
    unsigned long totMem; //vui.vui_total,   /* total memory */
    unsigned long comMem; //vui.vui_common,  /* common memory */
    unsigned long shMem;  //vui.vui_shared,  /* shared memory */
    char sleepState; //p_state,                            /* sleep state */
    int ppid; //ppid,                               /* parent PID */
    unsigned int realuid; //mproc[pi].mp_realuid,        /* real UID */
    unsigned int effuid; //mproc[pi].mp_effuid,      /* effective UID */
    unsigned int procgrp; //mproc[pi].mp_procgrp,    /* process group */
    int nice; //mproc[pi].mp_nice,                  /* nice value */
    char f_state; //f_state,                        /* VFS block state */
    int fp_blk_on; //(int)(fproc[pi].fp_blocked_on == FP_BLOCKED_ON_OTHER) ?
                   //fproc[pi].fp_task : NONE,       /* block proc */
    unsigned long long fp_tty; //fproc[pi].fp_tty   /* controlling tty */
    // END If not TYPE_TASK

    // ALL -------------
    unsigned long kipc_cycles_hi; // ex64hi(proc[i].p_kipc_cycles),
    unsigned long kipc_cycles_lo; // ex64lo(proc[i].p_kipc_cycles),
    uint64_t kipc_cycles;
    unsigned long kcall_cycles_hi; // ex64hi(proc[i].p_kcall_cycles),
    unsigned long kcall_cycles_lo; // ex64lo(proc[i].p_kcall_cycles));
    uint64_t kcall_cycles;
    // END ALL -------------

    // If TYPE_TASK
    unsigned long vui_total;

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
