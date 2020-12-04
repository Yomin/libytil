/*
 * Copyright (c) 2018-2020 Martin Rödel a.k.a. Yomin Nimoy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/// \file

#include <ytil/def/os.h>

#if OS_WINDOWS || DOXYGEN

#include <ytil/sys/service.h>
#include <ytil/gen/error.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <windows.h>


/// service state
typedef struct service_state
{
    const service_control_st    *control;   ///< service control handlers
    SERVICE_STATUS_HANDLE       handle;     ///< service handle to communicate with SCM
    SERVICE_STATUS              status;     ///< current service status
} service_state_st;

/// global service state
static service_state_st service;

/// service error type definition
ERROR_DEFINE_LIST(SERVICE,
    ERROR_INFO(E_SERVICE_CONTROL,        "Service control handler error."),
    ERROR_INFO(E_SERVICE_DEPENDENCY,     "Dependent services running."),
    ERROR_INFO(E_SERVICE_ENABLED,        "Service is enabled."),
    ERROR_INFO(E_SERVICE_DISABLED,       "Service is disabled."),
    ERROR_INFO(E_SERVICE_EXISTS,         "Service exists."),
    ERROR_INFO(E_SERVICE_INVALID_NAME,   "Invalid service name."),
    ERROR_INFO(E_SERVICE_INVALID_USER,   "Invalid service user."),
    ERROR_INFO(E_SERVICE_NOT_SERVICE,    "Caller is not a service."),
    ERROR_INFO(E_SERVICE_PATH_NOT_FOUND, "Service executable path not found."),
    ERROR_INFO(E_SERVICE_PATH_TOO_LONG,  "Service executable path is too long."),
    ERROR_INFO(E_SERVICE_PAUSED,         "Service is paused."),
    ERROR_INFO(E_SERVICE_RUNNING,        "Service is running."),
    ERROR_INFO(E_SERVICE_STOPPED,        "Service is stopped."),
    ERROR_INFO(E_SERVICE_TIMEOUT,        "Service control manager is busy."),
    ERROR_INFO(E_SERVICE_UNKNOWN,        "Unknown service."),
    ERROR_INFO(E_SERVICE_UNSUPPORTED,    "Unsupported service control action.")
);

/// default error type for service module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_SERVICE

int service_install(const char *name, const char *desc, const char *user, const char *password, bool autostart, const char *path, const char *args, ...)
{
    SERVICE_DESCRIPTION service_desc = { 0 };
    SERVICE_FAILURE_ACTIONS failure = { 0 };
    SC_ACTION actions[3] = { 0 };
    SC_HANDLE mgr, service;
    char buf[512 + 2];
    DWORD rc, len;
    va_list ap1, ap2;

    assert(name);

    if(path)
    {
        if((len = strlen(path)) > sizeof(buf) - 2)
            return error_set(E_SERVICE_PATH_TOO_LONG), -1;

        strncpy(buf, path, sizeof(buf));
    }
    else
    {
        if(!(len = GetModuleFileName(NULL, buf, sizeof(buf))))
            return error_wrap_last_ewin32(GetModuleFileName), -1;

        if(len > sizeof(buf) - 2)
            return error_set(E_SERVICE_PATH_TOO_LONG), -1;
    }

    if(args)
    {
        va_start(ap1, args);
        va_copy(ap2, ap1);

        if(len + 1 + vsnprintf(NULL, 0, args, ap2) > sizeof(buf) - 2)
            return error_set(E_SERVICE_PATH_TOO_LONG), va_end(ap2), va_end(ap1), -1;

        buf[len] = ' ';
        vsnprintf(buf + len + 1, sizeof(buf) - len - 1, args, ap1);

        va_end(ap2);
        va_end(ap1);
    }

    if(!(mgr = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE)))
        return error_wrap_last_ewin32(OpenSCManager), -1;

    service = CreateService(mgr, name, NULL,
        SERVICE_CHANGE_CONFIG | SERVICE_START,
        SERVICE_WIN32_OWN_PROCESS,
        autostart ? SERVICE_AUTO_START : SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL, buf, NULL, NULL, NULL, user, password);

    if(!service)
    {
        rc = GetLastError();
        CloseServiceHandle(mgr);

        switch(rc)
        {
        case ERROR_INVALID_NAME:
            return error_pack_ewin32(E_SERVICE_INVALID_NAME, CreateService, rc), -1;

        case ERROR_INVALID_SERVICE_ACCOUNT:
            return error_pack_ewin32(E_SERVICE_INVALID_USER, CreateService, rc), -1;

        case ERROR_DUPLICATE_SERVICE_NAME:
        case ERROR_SERVICE_EXISTS:
            return error_pack_ewin32(E_SERVICE_EXISTS, CreateService, rc), -1;

        default:
            return error_wrap_ewin32(CreateService, rc), -1;
        }
    }

    CloseServiceHandle(mgr);

    if(desc)
    {
        service_desc.lpDescription = (char *)desc;
        ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &service_desc);
    }

    failure.dwResetPeriod   = 60 * 60;
    failure.cActions        = 3;
    failure.lpsaActions     = actions;
    actions[0].Type         = SC_ACTION_RESTART;
    actions[1].Type         = SC_ACTION_RESTART;
    actions[2].Type         = SC_ACTION_NONE;

    ChangeServiceConfig2(service, SERVICE_CONFIG_FAILURE_ACTIONS, &failure);

    CloseServiceHandle(service);

    return 0;
}

/// Connect to service.
///
/// \param name     service name
/// \param access   access rights
///
/// \returns                                service handle
/// \retval NULL/E_SERVICE_INVALID_NAME     invalid service name
/// \retval NULL/E_SERVICE_UNKNOWN          unknown service
/// \retval NULL/E_GENERIC_WRAP             wrapped EWIN32 error
static SC_HANDLE service_connect(const char *name, DWORD access)
{
    SC_HANDLE mgr, service;
    DWORD rc;

    assert(name);

    if(!(mgr = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT)))
        return error_wrap_last_ewin32(OpenSCManager), NULL;

    if(!(service = OpenService(mgr, name, access)))
    {
        rc = GetLastError();
        CloseServiceHandle(mgr);

        switch(rc)
        {
        case ERROR_INVALID_NAME:
            return error_pack_ewin32(E_SERVICE_INVALID_NAME, OpenService, rc), NULL;

        case ERROR_SERVICE_DOES_NOT_EXIST:
            return error_pack_ewin32(E_SERVICE_UNKNOWN, OpenService, rc), NULL;

        default:
            return error_wrap_ewin32(OpenService, rc), NULL;
        }
    }

    CloseServiceHandle(mgr);

    return service;
}

int service_remove(const char *name, bool wait)
{
    SC_HANDLE service;
    DWORD rc;

    if(!(service = service_connect(name, DELETE)))
        return error_pass(), -1;

    if(!DeleteService(service) && (rc = GetLastError()) != ERROR_SERVICE_MARKED_FOR_DELETE)
        return error_wrap_ewin32(DeleteService, rc), CloseServiceHandle(service), -1;

    CloseServiceHandle(service);

    if(!wait)
        return 0;

    while((service = service_connect(name, DELETE)))
    {
        CloseServiceHandle(service);
        usleep(200);
    }

    return 0;
}

service_status_id service_status(const char *name)
{
    SC_HANDLE service;
    SERVICE_STATUS status;

    if(!(service = service_connect(name, SERVICE_QUERY_STATUS)))
        return error_pass(), SERVICE_STATUS_INVALID;

    if(!QueryServiceStatus(service, &status))
    {
        error_wrap_last_ewin32(QueryServiceStatus);
        CloseServiceHandle(service);

        return SERVICE_STATUS_INVALID;
    }

    CloseServiceHandle(service);

    switch(status.dwCurrentState)
    {
    case SERVICE_START_PENDING:
        return SERVICE_STATUS_STARTING;

    case SERVICE_RUNNING:
        return SERVICE_STATUS_RUNNING;

    case SERVICE_PAUSE_PENDING:
        return SERVICE_STATUS_PAUSING;

    case SERVICE_PAUSED:
        return SERVICE_STATUS_PAUSED;

    case SERVICE_CONTINUE_PENDING:
        return SERVICE_STATUS_CONTINUING;

    case SERVICE_STOP_PENDING:
        return SERVICE_STATUS_STOPPING;

    case SERVICE_STOPPED:
        return SERVICE_STATUS_STOPPED;

    default:
        abort();
    }
}

pid_t service_pid(const char *name)
{
    SC_HANDLE service;
    char buf[512];
    SERVICE_STATUS_PROCESS *status = (SERVICE_STATUS_PROCESS *)buf;
    DWORD rc, size = sizeof(buf), pid;

    if(!(service = service_connect(name, SERVICE_QUERY_STATUS)))
        return error_pass(), -1;

    if(!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (unsigned char *)status, size, &size))
    {
        if((rc = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
            return error_wrap_ewin32(QueryServiceStatusEx, rc), CloseServiceHandle(service), -1;

        if(!(status = malloc(size)))
            return error_wrap_last_errno(malloc), CloseServiceHandle(service), -1;

        if(!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (unsigned char *)status, size, &size))
            return error_wrap_last_ewin32(QueryServiceStatusEx), free(status), CloseServiceHandle(service), -1;
    }

    CloseServiceHandle(service);

    pid = status->dwProcessId;

    if(status != (SERVICE_STATUS_PROCESS *)buf)
        free(status);

    return pid;
}

int service_start(const char *name, int argc, char *argv[], bool wait)
{
    SC_HANDLE service;
    SERVICE_STATUS status;
    DWORD rc;

    assert(!argc || argv);

    if(!(service = service_connect(name, SERVICE_START | SERVICE_QUERY_STATUS)))
        return error_pass(), -1;

    if(!StartService(service, argc, (const char **)argv))
    {
        rc = GetLastError();

        if(rc == ERROR_SERVICE_ALREADY_RUNNING)
        {
            if(!QueryServiceStatus(service, &status))
                return error_wrap_last_ewin32(QueryServiceStatus), CloseServiceHandle(service), -1;

            CloseServiceHandle(service);

            if(status.dwCurrentState == SERVICE_PAUSED)
                return error_set(E_SERVICE_PAUSED), -1;
            else
                return error_pack_ewin32(E_SERVICE_RUNNING, StartService, rc), -1;
        }

        CloseServiceHandle(service);

        switch(rc)
        {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return error_pack_ewin32(E_SERVICE_PATH_NOT_FOUND, StartService, rc), -1;

        case ERROR_SERVICE_DISABLED:
            return error_pack_ewin32(E_SERVICE_DISABLED, StartService, rc), -1;

        case ERROR_SERVICE_REQUEST_TIMEOUT:
            return error_pack_ewin32(E_SERVICE_TIMEOUT, StartService, rc), -1;

        case ERROR_SERVICE_MARKED_FOR_DELETE:
            return error_pack_ewin32(E_SERVICE_UNKNOWN, StartService, rc), -1;

        // ERROR_SERVICE_LOGON_FAILED

        default:
            return error_wrap_ewin32(StartService, rc), -1;
        }
    }

    if(!wait)
        return CloseServiceHandle(service), 0;

    while(1)
    {
        if(!QueryServiceStatus(service, &status))
            return error_wrap_last_ewin32(QueryServiceStatus), CloseServiceHandle(service), -1;

        if(status.dwCurrentState == SERVICE_RUNNING)
            break;

        usleep(200);
    }

    CloseServiceHandle(service);

    return 0;
}

/// Control service.
///
/// \param name     service name
/// \param wait     wait until control action complete
/// \param access   access rights
/// \param control  control action
/// \param state    target state
/// \param error    error if already in target state
///
/// \retval 0                           success
/// \retval -1/E_SERVICE_INVALID_NAME   invalid service name
/// \retval -1/E_SERVICE_UNKNOWN        unknown service
/// \retval -1/E_SERVICE_UNSUPPORTED    unsupported control action
/// \retval -1/E_SERVICE_DEPENDENCY     dependent services running
/// \retval -1/E_SERVICE_STOPPED        service is stopped
/// \retval -1/E_SERVICE_TIMEOUT        service control manager is busy
/// \retval -1/E_GENERIC_WRAP           wrapped EWIN32 error
/// \retval -1/error                    service already in target state
static int service_control(const char *name, bool wait, DWORD access, DWORD control, DWORD state, int error)
{
    SC_HANDLE service;
    SERVICE_STATUS status;
    DWORD rc;

    if(!(service = service_connect(name, access | SERVICE_QUERY_STATUS)))
        return error_pass(), -1;

    if(!QueryServiceStatus(service, &status))
        return error_wrap_last_ewin32(QueryServiceStatus), CloseServiceHandle(service), -1;

    if(status.dwCurrentState == state)
        return error_set(error), CloseServiceHandle(service), -1;

    if(!ControlService(service, control, &status))
    {
        rc = GetLastError();
        CloseServiceHandle(service);

        switch(rc)
        {
        case ERROR_INVALID_SERVICE_CONTROL:
            return error_pack_ewin32(E_SERVICE_UNSUPPORTED, ControlService, rc), -1;

        case ERROR_DEPENDENT_SERVICES_RUNNING:
            return error_pack_ewin32(E_SERVICE_DEPENDENCY, ControlService, rc), -1;

        case ERROR_SERVICE_NOT_ACTIVE:
            return error_pack_ewin32(E_SERVICE_STOPPED, ControlService, rc), -1;

        case ERROR_SERVICE_REQUEST_TIMEOUT:
            return error_pack_ewin32(E_SERVICE_TIMEOUT, ControlService, rc), -1;

        default:
            return error_wrap_ewin32(ControlService, rc), -1;
        }
    }

    if(!wait)
        return CloseServiceHandle(service), 0;

    while(status.dwCurrentState != state)
    {
        usleep(200);

        if(!QueryServiceStatus(service, &status))
            return error_wrap_last_ewin32(QueryServiceStatus), CloseServiceHandle(service), -1;
    }

    CloseServiceHandle(service);

    return 0;
}

int service_pause(const char *name, bool wait)
{
    return error_pass_int(service_control(name, wait,
        SERVICE_PAUSE_CONTINUE, SERVICE_CONTROL_PAUSE, SERVICE_PAUSED, E_SERVICE_PAUSED));
}

int service_continue(const char *name, bool wait)
{
    return error_pass_int(service_control(name, wait,
        SERVICE_PAUSE_CONTINUE, SERVICE_CONTROL_CONTINUE, SERVICE_RUNNING, E_SERVICE_RUNNING));
}

int service_stop(const char *name, bool wait)
{
    return error_pass_int(service_control(name, wait,
        SERVICE_STOP, SERVICE_CONTROL_STOP, SERVICE_STOPPED, E_SERVICE_STOPPED));
}

int service_kill(const char *name)
{
    pid_t pid;
    HANDLE process;

    if((pid = service_pid(name)) < 0)
        return error_pass(), -1;

    if(!(process = OpenProcess(PROCESS_TERMINATE, FALSE, pid)))
        return error_wrap_last_ewin32(OpenProcess), -1;

    if(!TerminateProcess(process, 0))
        return error_wrap_last_ewin32(TerminateProcess), CloseHandle(process), -1;

    CloseHandle(process);

    return 0;
}

/// Get service start type.
///
/// \param      service     service handle
/// \param[out] type        start type to fill
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_WRAP   wrapped EWIN32 error
static int service_get_start_type(SC_HANDLE service, DWORD *type)
{
    char buf[512];
    QUERY_SERVICE_CONFIG *config = (QUERY_SERVICE_CONFIG *)buf;
    DWORD rc, size = sizeof(buf);

    if(!QueryServiceConfig(service, config, size, &size))
    {
        if((rc = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
            return error_wrap_ewin32(QueryServiceConfig, rc), -1;

        if(!(config = malloc(size)))
            return error_wrap_last_errno(malloc), -1;

        if(!QueryServiceConfig(service, config, size, &size))
            return error_wrap_last_ewin32(QueryServiceConfig), free(config), -1;
    }

    *type = config->dwStartType;

    if(config != (QUERY_SERVICE_CONFIG *)buf)
        free(config);

    return 0;
}

/// Set service start type.
///
/// \param name     service name
/// \param type     start type to set
/// \param error    error if service already has start type
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_WRAP   wrapped EWIN32 error
/// \retval -1/error            service already has start type
static int service_set_start_type(const char *name, DWORD type, int error)
{
    SC_HANDLE service;
    DWORD current_type;

    if(!(service = service_connect(name, SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG)))
        return error_pass(), -1;

    if(service_get_start_type(service, &current_type))
        return error_pass(), CloseServiceHandle(service), -1;

    if(current_type == type)
        return error_set(error), CloseServiceHandle(service), -1;

    if(!ChangeServiceConfig(service, SERVICE_NO_CHANGE, type, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
        return error_wrap_last_ewin32(ChangeServiceConfig), CloseServiceHandle(service), -1;

    CloseServiceHandle(service);

    return 0;
}

int service_disable(const char *name)
{
    return error_pass_int(service_set_start_type(name, SERVICE_DISABLED, E_SERVICE_DISABLED));
}

int service_enable(const char *name, bool autostart)
{
    return error_pass_int(service_set_start_type(name,
        autostart ? SERVICE_AUTO_START : SERVICE_DEMAND_START, E_SERVICE_ENABLED));
}

/// Update service status.
///
/// \param state    new service status
/// \param rc       exit code
static void service_update(DWORD state, int rc)
{
    service.status.dwCurrentState               = state;
    service.status.dwWin32ExitCode              = rc ? ERROR_SERVICE_SPECIFIC_ERROR : 0;
    service.status.dwServiceSpecificExitCode    = rc;

    SetServiceStatus(service.handle, &service.status); // ignore error
}

/// Service control handler. Called from service manager.
///
/// \param ctrl         control action
/// \param evt_type     action specific event type
/// \param evt_data     event data
/// \param ctx          context
///
/// \retval NO_ERROR                    success
/// \retval ERROR_CALL_NOT_IMPLEMENTED  control action not implemented
///
/// \implements LPHANDLER_FUNCTION_EX
static DWORD service_handle_control(DWORD ctrl, DWORD evt_type, void *evt_data, void *ctx)
{
    switch(ctrl)
    {
    case SERVICE_CONTROL_PAUSE:

        if(!service.control->pause)
            return ERROR_CALL_NOT_IMPLEMENTED;

        service_update(SERVICE_PAUSE_PENDING, 0);
        service.control->pause(true, service.control->ctx);
        service_update(SERVICE_PAUSED, 0);
        break;

    case SERVICE_CONTROL_CONTINUE:

        if(!service.control->pause)
            return ERROR_CALL_NOT_IMPLEMENTED;

        service_update(SERVICE_CONTINUE_PENDING, 0);
        service.control->pause(false, service.control->ctx);
        service_update(SERVICE_RUNNING, 0);
        break;

    case SERVICE_CONTROL_STOP:

        if(!service.control->stop)
            return ERROR_CALL_NOT_IMPLEMENTED;

        service.control->stop(service.control->ctx);
        break;

    case SERVICE_CONTROL_PARAMCHANGE:

        if(!service.control->reload)
            return ERROR_CALL_NOT_IMPLEMENTED;

        service.control->reload(service.control->ctx);
        break;

    case SERVICE_CONTROL_SHUTDOWN:

        if(!service.control->shutdown)
            return ERROR_CALL_NOT_IMPLEMENTED;

        service.control->shutdown(service.control->ctx);
        break;

    case SERVICE_CONTROL_POWEREVENT:

        if(!service.control->power)
            return ERROR_CALL_NOT_IMPLEMENTED;

        service.control->power(service.control->ctx);
        break;

    case SERVICE_CONTROL_SESSIONCHANGE:

        if(!service.control->session)
            return ERROR_CALL_NOT_IMPLEMENTED;

        service.control->session(service.control->ctx);
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    return NO_ERROR;
}

/// Service main. Called from dispatcher.
///
/// \param argc     number of arguments
/// \param argv     arguments
static void service_main(DWORD argc, char *argv[])
{
    int rc = 0;

    if(!(service.handle = RegisterServiceCtrlHandlerEx("", service_handle_control, NULL)))
    {
        service.status.dwWin32ExitCode = GetLastError();
        error_wrap_ewin32(RegisterServiceCtrlHandlerEx, service.status.dwWin32ExitCode);
        return;
    }

    service.status.dwServiceType    = SERVICE_WIN32_OWN_PROCESS;
    service.status.dwCheckPoint     = 0;
    service.status.dwWaitHint       = 5000;

    if(service.control->setup)
    {
        service_update(SERVICE_START_PENDING, 0);

        if((rc = error_pack_int(E_SERVICE_CONTROL, service.control->setup(argc, argv, service.control->ctx))))
            goto fail;
    }

    if(service.control->pause)
        service.status.dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;

    if(service.control->stop)
        service.status.dwControlsAccepted |= SERVICE_ACCEPT_STOP;

    if(service.control->reload)
        service.status.dwControlsAccepted |= SERVICE_ACCEPT_PARAMCHANGE;

    if(service.control->shutdown)
        service.status.dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;

    if(service.control->power)
        service.status.dwControlsAccepted |= SERVICE_ACCEPT_POWEREVENT;

    if(service.control->session)
        service.status.dwControlsAccepted |= SERVICE_ACCEPT_SESSIONCHANGE;

    if(service.control->main)
    {
        service_update(SERVICE_RUNNING, 0);

        rc = error_pack_int(E_SERVICE_CONTROL, service.control->main(service.control->ctx));
    }

fail:

    if(service.control->teardown)
    {
        service_update(SERVICE_STOP_PENDING, rc);
        service.control->teardown(service.control->ctx);
    }

    service_update(SERVICE_STOPPED, rc);
}

int service_dispatch(const service_control_st *control)
{
    const SERVICE_TABLE_ENTRY table[] = { { "", service_main }, { NULL, NULL } };
    DWORD rc;

    assert(control);

    service.control = control;

    if(!StartServiceCtrlDispatcher(table))
    {
        rc = GetLastError();

        if(rc == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
            return error_pack_ewin32(E_SERVICE_NOT_SERVICE, StartServiceCtrlDispatcher, rc), -1;

        return error_wrap_ewin32(StartServiceCtrlDispatcher, rc), -1;
    }

    if(service.status.dwWin32ExitCode)
        return error_pass(), -1;

    return 0;
}

#else // if OS_WINDOWS || DOXYGEN

typedef int dummy;

#endif // if OS_WINDOWS || DOXYGEN
