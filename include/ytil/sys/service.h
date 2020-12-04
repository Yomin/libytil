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

#ifndef YTIL_SYS_SERVICE_H_INCLUDED
#define YTIL_SYS_SERVICE_H_INCLUDED

#include <ytil/gen/error.h>
#include <sys/types.h>
#include <stdbool.h>


/// service error
typedef enum service_error
{
    E_SERVICE_CONTROL,          ///< wrapped control handler error
    E_SERVICE_DEPENDENCY,       ///< dependent services running
    E_SERVICE_DISABLED,         ///< service is disabled
    E_SERVICE_ENABLED,          ///< service is enabled
    E_SERVICE_EXISTS,           ///< service exists
    E_SERVICE_INVALID_NAME,     ///< invalid service name
    E_SERVICE_INVALID_USER,     ///< invalid service user
    E_SERVICE_NOT_SERVICE,      ///< caller ist not a service
    E_SERVICE_PATH_NOT_FOUND,   ///< service executable path not found
    E_SERVICE_PATH_TOO_LONG,    ///< service executable path too long
    E_SERVICE_PAUSED,           ///< service is paused
    E_SERVICE_RUNNING,          ///< service is running
    E_SERVICE_STOPPED,          ///< service is stopped
    E_SERVICE_TIMEOUT,          ///< service control manager is busy
    E_SERVICE_UNKNOWN,          ///< service is unknown
    E_SERVICE_UNSUPPORTED,      ///< service control action not supported
} service_error_id;

/// service error type declaration
ERROR_DECLARE(SERVICE);

/// service status
typedef enum service_status
{
    SERVICE_STATUS_INVALID,     ///< invalid status
    SERVICE_STATUS_STARTING,    ///< service is starting
    SERVICE_STATUS_RUNNING,     ///< service is running
    SERVICE_STATUS_PAUSING,     ///< service is pausing
    SERVICE_STATUS_PAUSED,      ///< service is paused
    SERVICE_STATUS_CONTINUING,  ///< service is continuing
    SERVICE_STATUS_STOPPING,    ///< service is stopping
    SERVICE_STATUS_STOPPED,     ///< service is stopped
} service_status_id;

/// Service setup callback.
///
/// Called before service main is executed.
/// Should setup everything to get service running.
///
/// \param argc     number of arguments
/// \param argv     argument list
/// \param ctx      callback context
///
/// \retval 0       success
/// \retval !=0     error
typedef int (*service_setup_cb)(int argc, char *argv[], void *ctx);

/// Service main callback.
///
/// \param ctx      callback context
///
/// \retval 0       success
/// \retval !=0     error
typedef int (*service_main_cb)(void *ctx);

/// Service teardown callback.
///
/// Called after service main has finished or an error occured.
///
/// \param ctx      callback context
typedef void (*service_teardown_cb)(void *ctx);

/// Service pause/continue callback.
///
/// \param pause    if true service must pause else continue
/// \param ctx      callback context
typedef void (*service_pause_cb)(bool pause, void *ctx);

/// Generic service control callback.
///
/// \param ctx      callback context
typedef void (*service_control_cb)(void *ctx);

/// service control handlers
typedef struct service_control
{
    service_setup_cb    setup;      ///< service setup
    service_main_cb     main;       ///< service main
    service_teardown_cb teardown;   ///< service teardown
    service_pause_cb    pause;      ///< service pause/continue handler
    service_control_cb  stop;       ///< service stop handler
    service_control_cb  reload;     ///< config reload handler
    service_control_cb  shutdown;   ///< OS shutdown handler
    service_control_cb  power;      ///< power event handler
    service_control_cb  session;    ///< session event handler
    void                *ctx;       ///< handler context
} service_control_st;


/// Install service.
///
/// \param name         name of service
/// \param desc         description of service, NULL for no description
/// \param user         user the service runs with, NULL for system
/// \param password     password for user, NULL if no password needed
/// \param autostart    true if service should autostart
/// \param path         path to executable, NULL for current executable
/// \param args         argument format string, NULL if no arguments to provide
/// \param ...          argument list
///
/// \retval 0                           success
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_INVALID_USER   invalid \p user
/// \retval -1/E_SERVICE_EXISTS         \p name is already registered for a service
/// \retval -1/E_SERVICE_PATH_TOO_LONG  executable path + arguments too long
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_install(const char *name, const char *desc, const char *user, const char *password, bool autostart, const char *path, const char *args, ...);

/// Remove service.
///
/// \param name     name of service to remove
/// \param wait     if true wait until service is removed
///
/// \retval 0                           success
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_remove(const char *name, bool wait);

/// Query service status.
///
/// \param name     name of service to query
///
/// \retval SERVICE_STATUS_STARTING                         service is starting
/// \retval SERVICE_STATUS_RUNNING                          service is running
/// \retval SERVICE_STATUS_PAUSING                          service is pausing
/// \retval SERVICE_STATUS_PAUSED                           service is paused
/// \retval SERVICE_STATUS_CONTINUING                       service is continuing
/// \retval SERVICE_STATUS_STOPPING                         service is stopping
/// \retval SERVICE_STATUS_STOPPED                          service is stopped
/// \retval SERVICE_STATUS_INVALID/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval SERVICE_STATUS_INVALID/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval SERVICE_STATUS_INVALID/E_GENERIC_WRAP           wrapped win32 error
service_status_id service_status(const char *name);

/// Get service PID.
///
/// \param name     name of service
///
/// \returns                            service PID
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
pid_t service_pid(const char *name);

/// Control service.
///
/// \param name     name of service to start
/// \param argc     number of arguments
/// \param argv     argument list
/// \param wait     if true wait until service is running
///
/// \retval 0                           success
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval -1/E_SERVICE_PATH_NOT_FOUND \p service executable path not found
/// \retval -1/E_SERVICE_DISABLED       service is disabled
/// \retval -1/E_SERVICE_TIMEOUT        service control manager is busy
/// \retval -1/E_SERVICE_RUNNING        service is already running
/// \retval -1/E_SERVICE_PAUSED         service is paused
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_start(const char *name, int argc, char *argv[], bool wait);

/// Pause service.
///
/// \param name     name of service to pause
/// \param wait     if true wait until service is paused
///
/// \retval 0                           success
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval -1/E_SERVICE_UNSUPPORTED    pause not supported by service
/// \retval -1/E_SERVICE_PAUSED         service is already paused
/// \retval -1/E_SERVICE_STOPPED        service is stopped
/// \retval -1/E_SERVICE_TIMEOUT        service control manager is busy
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_pause(const char *name, bool wait);

/// Continue paused service.
///
/// \param name     name of service to continue
/// \param wait     if true wait until service is running
///
/// \retval 0                           success
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval -1/E_SERVICE_UNSUPPORTED    pause not supported by service
/// \retval -1/E_SERVICE_RUNNING        service is already running
/// \retval -1/E_SERVICE_STOPPED        service is stopped
/// \retval -1/E_SERVICE_TIMEOUT        service control manager is busy
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_continue(const char *name, bool wait);

/// Stop service.
///
/// \param name     name of service to stop
/// \param wait     if true wait until service is stopped
///
/// \retval 0                           success
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval -1/E_SERVICE_UNSUPPORTED    stop not supported by service
/// \retval -1/E_SERVICE_DEPENDENCY     dependent services running
/// \retval -1/E_SERVICE_STOPPED        service is already stopped
/// \retval -1/E_SERVICE_TIMEOUT        service control manager is busy
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_stop(const char *name, bool wait);

/// Kill service process.
///
/// \param name     name of service to kill
///
/// \retval 0
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_kill(const char *name);

/// Disable service.
///
/// \param name     name of service to disable
///
/// \retval 0
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval -1/E_SERVICE_DISABLED       service is already disabled
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_disable(const char *name);

/// Enable service.
///
/// \param name         name of service to enable
/// \param autostart    true if service should autostart
///
/// \retval 0
/// \retval -1/E_SERVICE_INVALID_NAME   invalid \p name
/// \retval -1/E_SERVICE_UNKNOWN        \p name refers to an unknown service
/// \retval -1/E_SERVICE_ENABLED        service is already enabled
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_enable(const char *name, bool autostart);

/// Dispatch service. Called by services.
///
/// \param control  service handlers
///
/// \retval 0                           success
/// \retval -1/E_SERVICE_CONTROL        wrapped control handler error
/// \retval -1/E_SERVICE_NOT_SERVICE    caller is not a service
/// \retval -1/E_GENERIC_WRAP           wrapped win32 error
int service_dispatch(const service_control_st *control);


#endif // ifndef YTIL_SYS_SERVICE_H_INCLUDED

#endif // if OS_WINDOWS || DOXYGEN
