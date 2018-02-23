//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>

#include "winders.h"
#include "log.h"

#include <cstdlib>



static std::string unwide(const std::wstring &ws) {
    std::vector<char> rval(ws.size() + 1, '\0');

    size_t status = std::wcstombs(rval.data(), ws.data(), rval.size());
    if (status == static_cast<size_t>(-1)) {
        logProblem << "wcstombs() failed." << std::endl;
        return "";
    }
    rval.resize(status);

    return std::string(rval.begin(), rval.end());
}


std::vector<std::string> WinHardware::getAdapters(void) {
    ULONG outBufLen = 0;
    const ULONG flags = GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_UNICAST
        | GAA_FLAG_SKIP_MULTICAST;
    GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, nullptr, &outBufLen);

    std::vector<BYTE> addrBuf(outBufLen);
    std::vector<std::string> rval;

    if(GetAdaptersAddresses(AF_UNSPEC, flags, nullptr,
        reinterpret_cast<PIP_ADAPTER_ADDRESSES>(addrBuf.data()), &outBufLen)
      == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES pAddresses =
            reinterpret_cast<PIP_ADAPTER_ADDRESSES>(addrBuf.data());
        while (pAddresses) {
            const std::string desc = unwide(pAddresses->Description);
            if (pAddresses->IfType == IF_TYPE_ETHERNET_CSMACD
              || pAddresses->IfType == IF_TYPE_IEEE80211)
                rval.push_back(desc);

            pAddresses  = pAddresses->Next;
        }
    }
    else {
        logProblem << "Call to GetAdaptersAddresses failed." << std::endl;
    }

    return rval;
}
