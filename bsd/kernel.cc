//
//  NetBSD port:  
//  Copyright (c) 1995 Brian Grayson(bgrayson@pine.ece.utexas.edu)
//
// dummy device ignore code by : David Cuka (dcuka@intgp1.ih.att.com)
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: kernel.cc,v 1.4 1996/12/02 05:03:48 bgrayson Exp $
//
#include "general.h"
#include <stdio.h>
#include <fcntl.h>
#include <kvm.h>
#include <nlist.h>
#include <limits.h>		/*  For _POSIX2_LINE_MAX  */

#include <err.h>                /*  For err(), warn(), etc.  BCG  */
#include <sys/dkstat.h>         /*  For CPUSTATES, which tells us how
                                      many cpu states there are.  */
#include <sys/disk.h>		/*  For disk statistics.  */

#include <sys/socket.h>         /*  These two are needed for the  */
#include <net/if.h>             /*    NetMeter helper functions.  */


// ------------------------  local variables  ---------------------
kvm_t* kd = NULL;	//  This single kvm_t is shared by all
                        //  of the kvm routines.

//  This struct has the list of all the symbols we want from the kernel.
static struct nlist nlst[] =
{
{ "_cp_time" },
#define CP_TIME_SYM_INDEX 0
{ "_ifnet" },
#define IFNET_SYM_INDEX 1
{ "_disklist" },
#define DISKLIST_SYM_INDEX	2
  {NULL}
  };

static char kernelFileName[_POSIX2_LINE_MAX];

// ------------------------  utility functions  -------------------
//  The following is an error-checking form of kvm_read.  In addition
//  it uses kd as the implicit kernel-file to read.  Saves typing.
//  Since this is C++, it's an inline function rather than a macro.

static inline void
safe_kvm_read (u_long kernel_addr, void* user_addr, size_t nbytes)
{
    /*  Check for obvious bad symbols (i.e., from /netbsd when we
     *  booted off of /netbsd.old), such as symbols that reference
     *  0x00000000 (or anywhere in the first 256 bytes of memory).  */
  if ((kernel_addr&0xffffff00) == 0)
    errx(-1, "safe_kvm_read() was attempted on EA %#lx\n", kernel_addr);
  if (kvm_read (kd, kernel_addr, user_addr, nbytes)==-1)
    err(-1, "kvm_read() of kernel address %#lx", kernel_addr);
}

//  This version uses the symbol offset in the nlst variable, to make it
//  a little more convenient.  BCG
static inline void
safe_kvm_read_symbol (int nlstOffset, void* user_addr, size_t nbytes)
{
  safe_kvm_read (nlst[nlstOffset].n_value, user_addr, nbytes);
}


int
ValidSymbol (int index)
{
  return ((nlst[index].n_value & 0xffffff00) != 0);
}


void
NetBSDInit()
{
  kernelFileName[0] = '\0';
}

void
SetKernelName(const char* const kernelName)
{
  if (strlen(kernelName)>=_POSIX2_LINE_MAX)
  {
    fprintf (stderr, "Error:  kernel file name of '%s' is too long!\n",
      kernelName);
    exit(1);
  }
  strcpy(kernelFileName, kernelName);
}

void
OpenKDIfNeeded()
{
  char unusederrorstring[_POSIX2_LINE_MAX];

  if (kd) return; //  kd is non-NULL, so it has been initialized.  BCG

    /*  Open it read-only, for a little added safety.  */
    /*  If the first character of kernelFileName is not '\0', then use
	that kernel file.  Otherwise, use the default kernel, by
	specifying NULL.  */
  if ((kd = kvm_openfiles ((kernelFileName[0]) ? kernelFileName : NULL,
			    NULL, NULL, O_RDONLY, unusederrorstring))
      == NULL)
	  err (-1, "OpenKDIfNeeded():kvm-open()");
    /*  Now grab the symbol offsets for the symbols that we want.  */
  kvm_nlist (kd, nlst);

  //  Look at all of the returned symbols, and check for bad lookups.
  //  (This may be unnecessary, but better to check than not to...  )
  struct nlist * nlp = nlst;
  while (nlp && nlp->n_name) {
    if ((nlp->n_type == 0) || (nlp->n_value == 0))
      /*errx (-1, "kvm_nlist() lookup failed for symbol '%s'.", nlp->n_name);*/
      warnx ("kvm_nlist() lookup failed for symbol '%s'.", nlp->n_name);
    nlp++;
  }
}


// ------------------------  CPUMeter functions  ------------------

void
NetBSDCPUInit()
{
  OpenKDIfNeeded();
}

void
NetBSDGetCPUTimes (long* timeArray)
{
  if (!timeArray) errx (-1, "NetBSDGetCPUTimes():  passed pointer was null!\n");
  if (CPUSTATES != 5)
    errx (-1, "Error:  xosview for NetBSD expects 5 cpu states!\n");
  safe_kvm_read_symbol (CP_TIME_SYM_INDEX, timeArray, sizeof (long) * CPUSTATES);
}


// ------------------------  NetMeter functions  ------------------
void
NetBSDNetInit()
{
  OpenKDIfNeeded();
}

void
NetBSDGetNetInOut (long long * inbytes, long long * outbytes)
{
  struct ifnet * ifnetp;
  struct ifnet ifnet;
  //char ifname[256];

  //  The "ifnet" symbol in the kernel points to a 'struct ifnet' pointer.
  safe_kvm_read (nlst[IFNET_SYM_INDEX].n_value, &ifnetp, sizeof(ifnetp));

  *inbytes = 0;
  *outbytes = 0;

  while (ifnetp) {
    //  Now, dereference the pointer to get the ifnet struct.
    safe_kvm_read ((unsigned long) ifnetp, &ifnet, sizeof(ifnet));
#ifdef NET_DEBUG
    safe_kvm_read ((unsigned long) ifnet.if_name, ifname, 256);
    sprintf (ifname, "%s%d", ifname, ifnet.if_unit);
    printf ("Interface name is %s\n", ifname);
    printf ("Ibytes: %8ld Obytes %8ld\n", ifnet.if_ibytes, ifnet.if_obytes);
    printf ("Ipackets:  %8ld\n", ifnet.if_ipackets);
#endif
    *inbytes  += ifnet.if_ibytes;
    *outbytes += ifnet.if_obytes;

    //  Linked-list step taken from if.c in netstat source, line 120.
    ifnetp = (struct ifnet*) ifnet.if_list.tqe_next;
  }
}


/*  ---------------------- Swap Meter stuff  -----------------  */
int
NetBSDSwapInit() {
  OpenKDIfNeeded();
  /*  Need to merge some of swapinteral.cc here, to be smart about
   *  missing kvm symbols (due to OS version mismatch, for example).
   *  */
  /*return ValidSymbol(*/
  return 1;
}

/*  ---------------------- Disk Meter stuff  -----------------  */
int
NetBSDDiskInit() {
  OpenKDIfNeeded(); 
  return ValidSymbol(DISKLIST_SYM_INDEX);
}

void
NetBSDGetDiskXFerBytes (unsigned long long *bytesXferred)
{
  /*  This function is a little tricky -- we have to iterate over a
   *  list in kernel land.  To make things simpler, data structures
   *  and pointers for objects in kernel-land have kvm tacked on front
   *  of their names.  Thus, kvmdiskptr points to a disk struct in
   *  kernel memory.  kvmcurrdisk is a copy of the kernel's struct,
   *  and it has pointers in it to other structs, so it also is
   *  prefixed with kvm.  */
  struct disklist_head kvmdisklist;
  struct disk *kvmdiskptr;
  struct disk kvmcurrdisk;
  safe_kvm_read_symbol (DISKLIST_SYM_INDEX, &kvmdisklist, sizeof(kvmdisklist));
  kvmdiskptr = kvmdisklist.tqh_first;
  *bytesXferred = 0;
  while (kvmdiskptr != NULL)
  {
    safe_kvm_read ((unsigned int)kvmdiskptr, &kvmcurrdisk, sizeof(kvmcurrdisk));
      /*  Add up the contribution from this disk.  */
    *bytesXferred += kvmcurrdisk.dk_bytes;
#ifdef DEBUG
    printf ("Got %#x (lower 32bits)\n", (int) (*bytesXferred & 0xffffffff));
#endif
    kvmdiskptr = kvmcurrdisk.dk_link.tqe_next;
  }
}
  
