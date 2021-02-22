#include "service.hpp"

namespace emocheck {

LPQUERY_SERVICE_CONFIG GetWinServiceConfig(std::string srv_name) {
    LPQUERY_SERVICE_CONFIGW config;
    SC_HANDLE h_scm;
    SC_HANDLE h_svc;
    DWORD bufsize;
    BOOL res, err;

    h_scm = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (h_scm == 0)  // fail to get handle of service control manager
        return nullptr;

    h_svc = OpenServiceA(h_scm, srv_name.c_str(), SERVICE_QUERY_CONFIG);
    if (h_svc == 0)  // fail to get handle of service
        return nullptr;

    // get size of buffer (expects errorcode:122)
    res = QueryServiceConfigW(h_svc, nullptr, 0, &bufsize);
    err = GetLastError();
    if (err == ERROR_INSUFFICIENT_BUFFER) {
        config = (LPQUERY_SERVICE_CONFIGW)calloc(1, bufsize);
    } else {
        DBG("Fail to query service config. Servicename:" << srv_name << "\tError: " << err);
        CloseHandle(h_svc);
        return nullptr;
    }

    // query service config
    res = QueryServiceConfigW(h_svc, config, bufsize, &bufsize);
    if (!res) {
        err = GetLastError();
        DBG("Fail to query service config. Servicename:" << srv_name << "\tError: " << err);
        CloseHandle(h_svc);
        return nullptr;
    }
    CloseHandle(h_scm);
    CloseHandle(h_svc);
    return config;
}

std::vector<WinService> ListWinServices() {
    std::vector<WinService> services;
    SC_HANDLE h_scm;
    LPENUM_SERVICE_STATUSW lp_services;
    DWORD bufsize;
    DWORD cnt;

    h_scm = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (h_scm == 0)  // fail to get handle of service control manager
        return services;

    EnumServicesStatusW(
        h_scm,
        SERVICE_WIN32 | SERVICE_DRIVER,
        SERVICE_STATE_ALL,
        nullptr,
        0,
        &bufsize,
        &cnt,
        0);

    lp_services = (LPENUM_SERVICE_STATUSW)calloc(1, bufsize);

    EnumServicesStatusW(
        h_scm,
        SERVICE_WIN32 | SERVICE_DRIVER,
        SERVICE_STATE_ALL,
        lp_services,
        bufsize,
        &bufsize,
        &cnt,
        0);

    for (unsigned int idx = 0; idx < cnt; idx++) {
        WinService* svc = new WinService();
        svc->display_name = WideCharToString(lp_services[idx].lpDisplayName);
        svc->name = WideCharToString(lp_services[idx].lpServiceName);
        svc->state = lp_services[idx].ServiceStatus.dwCurrentState;
        svc->type = lp_services[idx].ServiceStatus.dwServiceType;
        services.push_back(*svc);
    }

    CloseHandle(h_scm);
    return services;
}

}  // namespace emocheck