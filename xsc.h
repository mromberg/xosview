//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef xsc_h
#define xsc_h

#include <string>
#include <vector>

#include <X11/SM/SMlib.h>

class IceClient;


class XSessionClient {
public:
    XSessionClient(int argc, char * const *argv,
      const std::string &sessionArg="--smid");
    ~XSessionClient(void);

    // returns false if no session manager found.
    bool init(void);

    // returns true if die message received.
    bool check(int waitMsec=0, bool all=false);

    bool die(void) const { return _die; }

    const std::string &sessionID(void) const { return _sessionID; }

    // For debug.  getProperties requests them.  A callback prints to cout.
    void getProperties(void);

private:
    bool _die;
    std::vector<std::string> _argv;
    const std::string _sessionArg;
    std::string _lastSessionID;
    std::string _sessionID;
    IceClient *_iceClient;
    SmcConn _smcConn;

    static std::string getuser(void);


    //---- from fsutil.h --
    static std::string cwd(void);
    static std::string abspath(const std::string &path);
    static std::string normpath(const std::string &path);
    static std::vector<std::string> split(const std::string& s,
      const std::string& delim, const bool keep_empty = true);
    static std::string join(const std::vector<std::string> &v,
      const std::string &sep);
    //---- from fsutil.h --


    // xsl protocol callbacks.
    static void saveCB(SmcConn smc_conn, void *client_data,
      int save_type, Bool shutdown, int interact_style, Bool fast);
    static void dieCB(SmcConn smc_conn, void *client_data);
    static void saveCompleteCB(SmcConn smc_conn, void *client_data);
    static void shutdownCancelledCB(SmcConn smc_conn, SmPointer client_data);
    static void propReplyCB(SmcConn smc_conn, SmPointer client_data,
      int num_props, SmProp **props);

    // Not implemented.
    XSessionClient(const XSessionClient &xsc);
    XSessionClient &operator=(const XSessionClient &rhs);
};

#endif
