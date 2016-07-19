//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "xsc.h"

#include <iostream>
#include <cstring>
#include <sstream>

#include <unistd.h>
#include <pwd.h>
#include <poll.h>
#include <X11/SM/SMlib.h>


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


XSessionClient::XSessionClient(int argc, char * const *argv,
  const std::string &sessionArg)
    : _die(false), _sessionArg(sessionArg),
    _iceClient(0), _smcConn(0) {

    _argv.reserve(argc);
    for (int i = 0 ; i < argc ; i++) {
        std::string sv(argv[i]);

        if (i == 0)
            sv = normpath(abspath(sv));

        if (sv == _sessionArg) {
            if (i + 1 >= argc)
                std::cerr << "missing " << _sessionArg << " argument."
                          << std::endl;
            else {
                _lastSessionID = std::string(argv[i + 1]);
                i++;
            }
        }
        else
            _argv.push_back(sv);
    }
}


XSessionClient::~XSessionClient(void) {

    if (_smcConn) {
        SmcCloseStatus status = SmcCloseConnection(_smcConn, 0, NULL);
        if (status == SmcClosedNow)
            ; // std::cerr << "SmcClosedNow\n";
        else if (status == SmcClosedASAP)
            std::cerr << "SmcClosedASAP\n";
        else if (status == SmcConnectionInUse)
            std::cerr << "SmcConnectionInUse\n";
        else
            std::cerr << "Unknown status: " << status << std::endl;
    }

    delete _iceClient;
}


bool XSessionClient::init(void) {

    _iceClient = new IceClient();
    if (!_iceClient->init())
        return false;

    // Setup params for SmcOpenConnection()
    std::vector<char> errorBuf(256, 0);
    char *clientID = 0;
    const char *prevID = _lastSessionID.size() ? _lastSessionID.c_str() : NULL;

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


bool XSessionClient::check(int waitMsec, bool all) {
    if (!_iceClient)
        return die();

    _iceClient->check(waitMsec, all);

    return die();
}


void XSessionClient::getProperties(void) {
    if (_smcConn)
        SmcGetProperties(_smcConn, propReplyCB, this);
}


void XSessionClient::propReplyCB(SmcConn smc_conn, SmPointer client_data,
  int num_props, SmProp **props) {

    // The crap in props is allocated and needs to be freed.
    (void)smc_conn;

    XSessionClient *xsc = static_cast<XSessionClient *>(client_data);

    std::cout << "=== Properties stored in session manager ===" << std::endl
              << "clientID: " << xsc->sessionID() << std::endl;

    for (int i = 0 ; i < num_props ; i++) {
        SmProp *p = props[i];
        std::cout << p->name
                  << " (" << p->type << "):"
                  << std::endl
                  << "    ";

        for (int j = 0 ; j < p->num_vals ; j++) {
            SmPropValue *v = &(p->vals[j]);
            std::string sv((char *)v->value, v->length);
            std::cout << sv << ((j != p->num_vals - 1) ? ", " : "\n");
        }
        SmFreeProperty(p);
    }
    free(props);
}


void XSessionClient::saveCB(SmcConn smc_conn, void *client_data,
  int save_type, Bool shutdown, int interact_style, Bool fast) {

    (void)save_type;
    (void)shutdown;
    (void)interact_style;
    (void)fast;

    std::cout << "saveCB()\n";

    XSessionClient *xsc = static_cast<XSessionClient *>(client_data);

    std::vector<XSVar> xsvars;
    // Mandatory.
    xsvars.push_back(XSVar(SmCloneCommand, xsc->_argv));
    XSVar resvar(SmRestartCommand, xsc->_argv);
    resvar.addVal(xsc->_sessionArg);
    resvar.addVal(xsc->_sessionID);
    xsvars.push_back(resvar);
    xsvars.push_back(XSVar(SmUserID, getuser()));
    xsvars.push_back(XSVar(SmProgram, normpath(abspath(xsc->_argv[0]))));
    // Optional.
    xsvars.push_back(XSVar(SmCurrentDirectory, cwd()));

    std::vector<SmProp *> propsp(xsvars.size(), 0);
    for (size_t i = 0 ; i < xsvars.size() ; i++)
        propsp[i] = xsvars[i].prop();

    SmcSetProperties(smc_conn, propsp.size(), propsp.data());
    SmcSaveYourselfDone(smc_conn, true);
}


void XSessionClient::dieCB(SmcConn smc_conn, void *client_data) {

    (void)smc_conn;

    std::cout << "dieCB()\n";

    XSessionClient *xsc = static_cast<XSessionClient *>(client_data);
    xsc->_die = true;
}


void XSessionClient::saveCompleteCB(SmcConn smc_conn, void *client_data) {

    (void)smc_conn;
    (void)client_data;

    std::cout << "saveCompleteCB()\n";
}


void XSessionClient::shutdownCancelledCB(SmcConn smc_conn,
  SmPointer client_data) {

    (void)smc_conn;
    (void)client_data;

    std::cout << "shutdownCancelledCB()\n";
}


std::string XSessionClient::getuser(void) {
    uid_t uid = getuid();
    struct passwd *pwstr = getpwuid(uid);

    if (pwstr)
        return pwstr->pw_name;

    std::ostringstream os;
    os << uid;
    return os.str();
}


std::string XSessionClient::cwd(void) {
    std::string result(1024,'\0');
    while( getcwd(&result[0], result.size()) == 0) {
        if( errno != ERANGE ) {
            std::cerr << "getcwd() failed: " << strerror(errno) << std::endl;
            return ".";
        }
        result.resize(result.size()*2);
    }
    result.resize(result.find('\0'));
    return result;
}


std::string XSessionClient::abspath(const std::string &path) {
    if (path.size()) {
        if (path[0] == '/')
            return path;

        size_t fpos = path.find('/');
        if (fpos != std::string::npos)
            return cwd() + '/' + path;
    }

    return path;
}

std::string XSessionClient::normpath(const std::string &path) {
    std::vector<std::string> comps = XSessionClient::split(path, "/");

    // remember if this is an absolute path for later
    // when we collapase ".."
    bool isabspath = false;
    if (comps.size() && comps[0] == "")
        isabspath = true;

    // First, remove all "" entries (which result from //)
    // and any "." entries.
    std::vector<std::string> filtered;
    for (size_t i = 0 ; i < comps.size() ; i++)
        if (comps[i] != "" && comps[i] != ".")
            filtered.push_back(comps[i]);

    // Now collapse the ".." items.
    comps.resize(0);
    for (size_t i = 0 ; i < filtered.size() ; i++) {
        if (filtered[i] != "..")
            comps.push_back(filtered[i]);
        else {
            // maybe collapse it
            if (comps.empty()) {
                // the root dir has a .. that points at /
                // If we don't know for sure we are at / (isabspath)
                // then keep the "..".  We don't use cwd() here.
                if (!isabspath)
                    comps.push_back(filtered[i]);

            }
            else {
                // There is something before the current ".." see if it
                // can be collapsed.
                if (comps.back() != "..") { // it got there for a reason
                    comps.pop_back();
                }
            }
        }
    }

    std::string rval = XSessionClient::join(comps, "/");
    if (isabspath)
        rval = "/" + rval;

    return rval;
}

#include <algorithm>
std::vector<std::string> XSessionClient::split(const std::string& s,
  const std::string& delim, const bool keep_empty) {
    std::vector<std::string> result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    std::string::const_iterator substart = s.begin(), subend;
    while (true) {
        subend = std::search(substart, s.end(), delim.begin(), delim.end());
        std::string temp(substart, subend);
        if (keep_empty || !temp.empty())
            result.push_back(temp);
        if (subend == s.end())
            break;

        substart = subend + delim.size();
    }

    return result;
}


std::string XSessionClient::join(const std::vector<std::string> &v,
  const std::string &sep) {
    std::string rval;
    for (size_t i = 0 ; i < v.size() ; i ++) {
        rval += v[i];
        if (i + 1 < v.size())
            rval += sep;
    }

    return rval;
}

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
    else {
        // error (errno set).
        std::cerr << "poll() failed\n";
    }

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
        std::cerr << "IceProcessMessagesIOError\n";
        IceCloseConnection(_iceConn);
        _iceConn = 0;
    }
    else if (status == IceProcessMessagesConnectionClosed) {
        std::cerr << "IceProcessMessagesConnectionClosed\n";
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
