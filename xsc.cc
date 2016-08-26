//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "xsc.h"
#include "configxosv.h"
#include "fsutil.h"
#include "log.h"
#include "strutil.h"

#include <iostream>
#include <sstream>
#include <cerrno>


#ifdef HAVE_LIB_SM

#include <X11/SM/SMlib.h>
#include <unistd.h>
#include <pwd.h>
#include <poll.h>

class IceClient {
public:
    IceClient(void);
    ~IceClient(void);

    bool init(void);

    int fd(void) const {
        return _iceConn ? IceConnectionNumber(_iceConn) : -1;
    }

    bool ready(int waitMsec=0);
    void check(int waitMsec=0, bool all=false);

private:
    IceConn _iceConn;

    void process(void);

    static void addConnCB(IceConn ice_conn, IcePointer client_data,
      Bool opening, IcePointer *watch_data);

    // Not implemented.
    IceClient(const IceClient &ic);
    IceClient &operator=(const IceClient &rhs);
};


class XSCImp {
public:
    XSCImp(int argc, char * const *argv,
      const std::string &sessionArg="--smid");
    ~XSCImp(void);

    // returns false if no session manager found.
    bool init(void);

    // returns true if die message received.
    bool check(int waitMsec=0, bool all=false);

    bool die(void) const { return _die; }

    const std::string &sessionID(void) const { return _sessionID; }

private:
    bool _die;
    std::vector<std::string> _argv;
    const std::string _sessionArg;
    std::string _lastSessionID;
    std::string _sessionID;
    IceClient *_iceClient;
    SmcConn _smcConn;

    static std::string getuser(void);

    // xsl protocol callbacks.
    static void saveCB(SmcConn smc_conn, void *client_data,
      int save_type, Bool shutdown, int interact_style, Bool fast);
    static void dieCB(SmcConn smc_conn, void *client_data);
    static void saveCompleteCB(SmcConn smc_conn, void *client_data);
    static void shutdownCancelledCB(SmcConn smc_conn, SmPointer client_data);
    static void propReplyCB(SmcConn smc_conn, SmPointer client_data,
      int num_props, SmProp **props);

    // For debug.  getProperties requests them.
    // A callback prints to logDebug.
    void getProperties(void);

    // Not implemented.
    XSCImp(const XSCImp &xsci);
    XSCImp &operator=(const XSCImp &rhs);
};


class XSVar {
public:
    XSVar(const std::string &name, const std::vector<std::string> &values);
    XSVar(const std::string &name, const std::string &value);

    void addVal(const std::string &v) { _values.push_back(v); }
    std::vector<std::string> values(void) { return _values; }

    SmProp *prop(void);

private:
    std::string _name;
    std::string _type;
    std::vector<std::string> _values;
    SmProp _prop;
    std::vector<SmPropValue> _pvals;
};

#endif


//-----------------------------------------------------------------------
// XSessionClient
//-----------------------------------------------------------------------
XSessionClient::XSessionClient(int argc, char * const *argv,
  const std::string &sessionArg) : _imp(0) {
#ifndef HAVE_LIB_SM
    (void)argc;
    (void)argv;
    (void)sessionArg;
#else
    _imp = new XSCImp(argc, argv, sessionArg);
#endif
}


XSessionClient::~XSessionClient(void) {
#ifdef HAVE_LIB_SM
    delete _imp;
#endif
}


bool XSessionClient::init(void) {
#ifdef HAVE_LIB_SM
    return _imp->init();
#else
    return false;
#endif
}


bool XSessionClient::check(int waitMsec, bool all) {
#ifndef HAVE_LIB_SM
    (void)waitMsec;
    (void)all;

    return false;
#else
    return _imp->check(waitMsec, all);
#endif
}


bool XSessionClient::die(void) const {
#ifdef HAVE_LIB_SM
    return _imp->die();
#else
    return false;
#endif
}


const std::string &XSessionClient::sessionID(void) const {
#ifdef HAVE_LIB_SM
    return _imp->sessionID();
#else
    static std::string nullid;
    return nullid;
#endif
}



#ifdef HAVE_LIB_SM
//-------------------------------------------------------------------
// IceClient
//-------------------------------------------------------------------
IceClient::IceClient(void) : _iceConn(0) {
}


IceClient::~IceClient(void) {
    if (_iceConn) {
        IceRemoveConnectionWatch(addConnCB, this);
        IceCloseConnection(_iceConn);
        _iceConn = 0;
    }
}


bool IceClient::init(void) {
    if (!IceAddConnectionWatch(addConnCB, this))
        return false;

    return true;
}


bool IceClient::ready(int waitMsec) {
    if (fd() == -1)
        return false;

    struct pollfd pfd;
    pfd.fd = fd();
    pfd.events = POLLIN;
    pfd.revents = 0;

    int presult = poll(&pfd, 1, waitMsec);

    if (presult > 0) {
        // sucess (presult == num with events/errors)
        return true;
    }
    else if (presult == 0) {
        // timeout.
    }
    // presult < 0 error occurred.
    else if (errno == EINTR)  // interrupted system call.
        return false;
    else
        logFatal << "poll() failed: " << util::strerror() << std::endl;

    return false;
}


void IceClient::check(int waitMsec, bool all) {
    if (all)
        while (ready())
            process();

    if (ready(waitMsec))
        process();
}


void IceClient::process(void) {
    if (!_iceConn)
        return;

    IceProcessMessagesStatus status = IceProcessMessages(_iceConn,
      NULL, NULL);

    if (status == IceProcessMessagesSuccess)
        ; // it worked.
    else if (status == IceProcessMessagesIOError) {
        logProblem << "IceProcessMessagesIOError" << std::endl;
        IceCloseConnection(_iceConn);
        _iceConn = 0;
    }
    else if (status == IceProcessMessagesConnectionClosed) {
        logProblem << "IceProcessMessagesConnectionClosed" << std::endl;
        _iceConn = 0;
    }
}


void IceClient::addConnCB(IceConn ice_conn, IcePointer client_data,
  Bool opening, IcePointer *watch_data) {

    (void)watch_data;

    IceClient *ic = static_cast<IceClient *>(client_data);

    ic->_iceConn = opening ? ice_conn : 0;
}


//-------------------------------------------------------------------
// XSVar
//-------------------------------------------------------------------
XSVar::XSVar(const std::string &name, const std::vector<std::string> &values)
    : _name(name), _type(SmLISTofARRAY8), _values(values) {
}


XSVar::XSVar(const std::string &name, const std::string &value)
    : _name(name), _type(SmARRAY8), _values(1, value) {
}


SmProp *XSVar::prop(void) {
    _prop.name = const_cast<char *>(_name.c_str());
    _prop.type = const_cast<char *>(_type.c_str());
    _pvals.clear();
    for (size_t i = 0 ; i < _values.size() ; i++) {
        SmPropValue pv;
        pv.length = _values[i].size();
        pv.value = (SmPointer)_values[i].c_str();
        _pvals.push_back(pv);
    }
    _prop.num_vals = _pvals.size();
    _prop.vals = const_cast<SmPropValue *>(_pvals.data());

    return &_prop;
}


//------------------------------------------------------------------------
// XSCImp
//------------------------------------------------------------------------
XSCImp::XSCImp(int argc, char * const *argv,
  const std::string &sessionArg)
    : _die(false), _sessionArg(sessionArg), _iceClient(0), _smcConn(0) {

    _argv.reserve(argc);
    for (int i = 0 ; i < argc ; i++) {
        std::string sv(argv[i]);

        if (i == 0)
            sv = util::fs::normpath(util::fs::abspath(sv));

        if (sv == _sessionArg) {
            if (i + 1 >= argc) {
                logProblem << "missing " << _sessionArg << " argument."
                           << std::endl;
            }
            else {
                _lastSessionID = std::string(argv[i + 1]);
                i++;
            }
        }
        else
            _argv.push_back(sv);
    }
}


XSCImp::~XSCImp(void) {
    if (_smcConn) {
        SmcCloseStatus status = SmcCloseConnection(_smcConn, 0, NULL);
        if (status == SmcClosedNow) {
            logDebug << "SmcClosedNow" << std::endl;
        }
        else if (status == SmcClosedASAP) {
            logDebug << "SmcClosedASAP" << std::endl;
        }
        else if (status == SmcConnectionInUse) {
            logDebug << "SmcConnectionInUse" << std::endl;
        }
        else
            logProblem << "Unknown status: " << status << std::endl;
    }

    delete _iceClient;
}


bool XSCImp::init(void) {
    _iceClient = new IceClient();
    if (!_iceClient->init())
        return false;

    // Setup params for SmcOpenConnection()
    std::vector<char> errorBuf(256, 0);
    char *clientID = NULL;
    char *prevID = NULL;
    if (_lastSessionID.size())
        prevID = const_cast<char *>(_lastSessionID.c_str());

    SmcCallbacks callbacks;
    callbacks.save_yourself.callback = saveCB;
    callbacks.save_yourself.client_data = this;
    callbacks.die.callback = dieCB;
    callbacks.die.client_data = this;
    callbacks.save_complete.callback = saveCompleteCB;
    callbacks.save_complete.client_data = this;
    callbacks.shutdown_cancelled.callback = shutdownCancelledCB;
    callbacks.shutdown_cancelled.client_data = this;

    unsigned long mask = SmcSaveYourselfProcMask |
        SmcDieProcMask |
        SmcSaveCompleteProcMask |
        SmcShutdownCancelledProcMask;

    _smcConn = SmcOpenConnection(NULL, NULL, SmProtoMajor, SmProtoMinor,
      mask, &callbacks, prevID, &clientID,
      errorBuf.size() - 1, errorBuf.data());

    if (!_smcConn)
        return false;

    _sessionID = std::string(clientID);
    free(clientID);

    return true;
}


bool XSCImp::check(int waitMsec, bool all) {
    if (!_iceClient)
        return die();

    _iceClient->check(waitMsec, all);
    return die();
}


void XSCImp::propReplyCB(SmcConn smc_conn, SmPointer client_data,
  int num_props, SmProp **props) {

    // The crap in props is allocated and needs to be freed.
    (void)smc_conn;

    XSCImp *xsc = static_cast<XSCImp *>(client_data);

    logDebug << "=== Properties stored in session manager ===" << std::endl;
    logDebug << "clientID: " << xsc->sessionID() << std::endl;

    for (int i = 0 ; i < num_props ; i++) {
        SmProp *p = props[i];

        std::ostringstream os;
        for (int j = 0 ; j < p->num_vals ; j++) {
            SmPropValue *v = &(p->vals[j]);
            std::string sv((char *)v->value, v->length);
            os << sv << ((j != p->num_vals - 1) ? ", " : "");
        }

        logDebug << p->name
                 << " (" << p->type << "):"
                 << std::endl;
        logDebug << "    " << os.str() << std::endl;

        SmFreeProperty(p);
    }
    free(props);
}


void XSCImp::saveCB(SmcConn smc_conn, void *client_data,
  int save_type, Bool shutdown, int interact_style, Bool fast) {

    (void)save_type;
    (void)shutdown;
    (void)interact_style;
    (void)fast;

    logDebug << "saveCB()" << std::endl;

    XSCImp *xsc = static_cast<XSCImp *>(client_data);

    std::vector<XSVar> xsvars;
    // Mandatory.
    xsvars.push_back(XSVar(SmCloneCommand, xsc->_argv));
    XSVar resvar(SmRestartCommand, xsc->_argv);
    resvar.addVal(xsc->_sessionArg);
    resvar.addVal(xsc->_sessionID);
    xsvars.push_back(resvar);
    xsvars.push_back(XSVar(SmUserID, getuser()));
    xsvars.push_back(XSVar(SmProgram,
        util::fs::normpath(util::fs::abspath(xsc->_argv[0]))));
    // Optional.
    xsvars.push_back(XSVar(SmCurrentDirectory, util::fs::cwd()));

    std::vector<SmProp *> propsp(xsvars.size(), 0);
    for (size_t i = 0 ; i < xsvars.size() ; i++)
        propsp[i] = xsvars[i].prop();

    SmcSetProperties(smc_conn, propsp.size(), propsp.data());
    SmcSaveYourselfDone(smc_conn, true);
}


void XSCImp::dieCB(SmcConn smc_conn, void *client_data) {

    (void)smc_conn;

    logDebug << "dieCB()" << std::endl;

    XSCImp *xsc = static_cast<XSCImp *>(client_data);
    xsc->_die = true;
}


void XSCImp::saveCompleteCB(SmcConn smc_conn, void *client_data) {

    (void)smc_conn;
    (void)client_data;

    logDebug << "saveCompleteCB()" << std::endl;

#ifdef XOSVDEBUG
    XSCImp *xsc = static_cast<XSCImp *>(client_data);
    xsc->getProperties();
#endif
}


void XSCImp::shutdownCancelledCB(SmcConn smc_conn,
  SmPointer client_data) {

    (void)smc_conn;
    (void)client_data;

    logDebug << "shutdownCancelledCB()" << std::endl;
}


std::string XSCImp::getuser(void) {
    uid_t uid = getuid();
    struct passwd *pwstr = getpwuid(uid);

    if (pwstr)
        return pwstr->pw_name;

    std::ostringstream os;
    os << uid;
    return os.str();
}

void XSCImp::getProperties(void) {
    if (_smcConn)
        SmcGetProperties(_smcConn, propReplyCB, this);
}


#endif // HAVE_LIB_SM
