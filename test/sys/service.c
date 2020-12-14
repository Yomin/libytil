/*
 * Copyright (c) 2020 Martin Rödel a.k.a. Yomin Nimoy
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

#include <ytil/def/os.h>

#if OS_WINDOWS

#include "sys.h"
#include <ytil/test/run.h>
#include <ytil/test/test.h>
#include <ytil/sys/service.h>
#include <unistd.h>
#include <windows.h>
#include <shlobj.h>

#define NAME    "ytil_test_service"
#define DESC    "test service"
#define USER    "NT AUTHORITY\\LocalService"
#define PATH    "foo\\bar\\baz.exe"
#define ARGS    "service abort"

static unsigned char buf1[500], buf2[100], buf3[100];
static QUERY_SERVICE_CONFIG *config     = (void *)buf1;
static SERVICE_DESCRIPTION *desc        = (void *)buf2;
static SERVICE_FAILURE_ACTIONS *actions = (void *)buf3;


static int service_query(void)
{
    SC_HANDLE mgr, service;
    DWORD size;

    if(!(mgr = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT)))
        return error_wrap_last_ewin32(OpenSCManager), -1;

    if(!(service = OpenService(mgr, NAME, SERVICE_QUERY_CONFIG)))
        return error_wrap_last_ewin32(OpenService), CloseServiceHandle(mgr), -1;

    CloseServiceHandle(mgr);

    if(!QueryServiceConfig(service, config, sizeof(buf1), &size))
        return error_wrap_last_ewin32(QueryServiceConfig), CloseServiceHandle(service), -1;

    if(!QueryServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, buf2, sizeof(buf2), &size))
        return error_wrap_last_ewin32(QueryServiceConfig2), CloseServiceHandle(service), -1;

    if(!QueryServiceConfig2(service, SERVICE_CONFIG_FAILURE_ACTIONS, buf3, sizeof(buf3), &size))
        return error_wrap_last_ewin32(QueryServiceConfig2), CloseServiceHandle(service), -1;

    CloseServiceHandle(service);

    return 0;
}

TEST_PSETUP(service_install, const char *user, const char *path, const char *args)
{
    test_int_maybe(service_remove(NAME, true), E_SERVICE_UNKNOWN);
    test_int_success(service_install(NAME, DESC, user, NULL, false, path, args));
}

TEST_TEARDOWN(service_remove)
{
    test_int_success(service_remove(NAME, true));
}

TEST_TEARDOWN(service_stop_remove)
{
    test_int_maybe(service_stop(NAME, true), E_SERVICE_STOPPED);
    test_int_success(service_remove(NAME, true));
}

TEST_TEARDOWN(service_kill_remove)
{
    test_int_success(service_remove(NAME, false));
    test_int_success(service_kill(NAME));
}

TEST_CASE_ABORT(service_install_invalid_name1)
{
    service_install(NULL, NULL, NULL, NULL, false, NULL, NULL);
}

TEST_CASE(service_install_invalid_name2)
{
    test_int_error(service_install("", NULL, NULL, NULL, false, NULL, NULL), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_install_invalid_user)
{
    test_int_maybe(service_remove(NAME, true), E_SERVICE_UNKNOWN);
    test_int_error(service_install(NAME, DESC, "foobarbaz", NULL, false, PATH, NULL), E_SERVICE_INVALID_USER);
}

TEST_CASE_PFIX(service_install, service_install, service_remove, USER, PATH, NULL)
{
    test_int_success(service_query());

    test_int_eq(config->dwServiceType, SERVICE_WIN32_OWN_PROCESS);
    test_int_eq(config->dwStartType, SERVICE_DEMAND_START);
    test_int_eq(config->dwErrorControl, SERVICE_ERROR_NORMAL);
    test_str_eq(config->lpBinaryPathName, PATH);
    test_str_eq(config->lpServiceStartName, USER);
    test_str_eq(config->lpDisplayName, NAME);
    test_str_eq(desc->lpDescription, DESC);
    test_int_eq(actions->dwResetPeriod, 60 * 60);
    test_ptr_eq(actions->lpRebootMsg, NULL);
    test_ptr_eq(actions->lpCommand, NULL);
    test_int_eq(actions->cActions, 3);
    test_int_eq(actions->lpsaActions[0].Type, SC_ACTION_RESTART);
    test_int_eq(actions->lpsaActions[0].Delay, 0);
    test_int_eq(actions->lpsaActions[1].Type, SC_ACTION_RESTART);
    test_int_eq(actions->lpsaActions[1].Delay, 0);
    test_int_eq(actions->lpsaActions[2].Type, SC_ACTION_NONE);
    test_int_eq(actions->lpsaActions[3].Delay, 0);
}

TEST_CASE_PFIX(service_install_system, service_install, service_remove, NULL, PATH, NULL)
{
    test_int_success(service_query());
    test_str_eq(config->lpServiceStartName, "LocalSystem");
}

TEST_CASE_PFIX(service_install_current_exe, service_install, service_remove, NULL, NULL, NULL)
{
    char buf[512];

    test_int_success(service_query());
    test_int_success_ewin32(GetModuleFileName(NULL, buf, sizeof(buf)) ? 0 : -1);
    test_str_eq(config->lpBinaryPathName, buf);
}

TEST_CASE_PFIX(service_install_args, service_install, service_remove, NULL, PATH, ARGS)
{
    test_int_success(service_query());
    test_str_eq(config->lpBinaryPathName, PATH " " ARGS);
}

TEST_CASE_PFIX(service_install_twice, service_install, service_remove, NULL, PATH, NULL)
{
    test_int_error(service_install(NAME, DESC, NULL, NULL, false, PATH, NULL), E_SERVICE_EXISTS);
}

TEST_CASE_ABORT(service_remove_invalid_name1)
{
    service_remove(NULL, false);
}

TEST_CASE(service_remove_invalid_name2)
{
    test_int_error(service_remove("", false), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_remove_unknown)
{
    test_int_error(service_remove("foobarbaz", false), E_SERVICE_UNKNOWN);
}

TEST_CASE_ABORT(service_status_invalid_name1)
{
    service_status(NULL);
}

TEST_CASE(service_status_invalid_name2)
{
    test_rc_error(service_status(""), SERVICE_STATUS_INVALID, E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_status_unknown)
{
    test_rc_error(service_status("foobarbaz"), SERVICE_STATUS_INVALID, E_SERVICE_UNKNOWN);
}

TEST_CASE_ABORT(service_pid_invalid_name1)
{
    service_pid(NULL);
}

TEST_CASE(service_pid_invalid_name2)
{
    test_int_error(service_pid(""), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_pid_unknown)
{
    test_int_error(service_pid("foobarbaz"), E_SERVICE_UNKNOWN);
}

TEST_CASE_PFIX(service_pid, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_success(service_pid(NAME));
}

TEST_CASE_ABORT(service_start_invalid_name1)
{
    service_start(NULL, 0, NULL, true);
}

TEST_CASE(service_start_invalid_name2)
{
    test_int_error(service_start("", 0, NULL, true), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_start_unknown)
{
    test_int_error(service_start("foobarbaz", 0, NULL, true), E_SERVICE_UNKNOWN);
}

TEST_CASE_PFIX(service_start_path_not_found, service_install, service_stop_remove, USER, PATH, NULL)
{
    test_int_error(service_start(NAME, 0, NULL, true), E_SERVICE_PATH_NOT_FOUND);
}

TEST_CASE_PFIX(service_start_disabled, service_install, service_stop_remove, NULL, NULL, "service disabled")
{
    test_int_success(service_disable(NAME));
    test_int_error(service_start(NAME, 0, NULL, true), E_SERVICE_DISABLED);
}

TEST_CASE_PFIX(service_start_stopped, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_rc_success(service_status(NAME), SERVICE_STATUS_RUNNING, SERVICE_STATUS_INVALID);
}

TEST_CASE_PFIX(service_start_paused, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_success(service_pause(NAME, true));
    test_int_error(service_start(NAME, 0, NULL, true), E_SERVICE_PAUSED);
}

TEST_CASE_PFIX(service_start_running, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_error(service_start(NAME, 0, NULL, true), E_SERVICE_RUNNING);
}

TEST_CASE_ABORT(service_pause_invalid_name1)
{
    service_pause(NULL, true);
}

TEST_CASE(service_pause_invalid_name2)
{
    test_int_error(service_pause("", true), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_pause_unknown)
{
    test_int_error(service_pause("foobarbaz", true), E_SERVICE_UNKNOWN);
}

TEST_CASE_PFIX(service_pause_unsupported, service_install, service_stop_remove, NULL, NULL, "service nopause")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_error(service_pause(NAME, true), E_SERVICE_UNSUPPORTED);
}

TEST_CASE_PFIX(service_pause_stopped, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_error(service_pause(NAME, true), E_SERVICE_STOPPED);
}

TEST_CASE_PFIX(service_pause_paused, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_success(service_pause(NAME, true));
    test_int_error(service_pause(NAME, true), E_SERVICE_PAUSED);
}

TEST_CASE_PFIX(service_pause_running, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_success(service_pause(NAME, true));
    test_rc_success(service_status(NAME), SERVICE_STATUS_PAUSED, SERVICE_STATUS_INVALID);
}

TEST_CASE_ABORT(service_continue_invalid_name1)
{
    service_continue(NULL, true);
}

TEST_CASE(service_continue_invalid_name2)
{
    test_int_error(service_continue("", true), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_continue_unknown)
{
    test_int_error(service_continue("foobarbaz", true), E_SERVICE_UNKNOWN);
}

TEST_CASE_PFIX(service_continue_stopped, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_error(service_continue(NAME, true), E_SERVICE_STOPPED);
}

TEST_CASE_PFIX(service_continue_paused, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_success(service_pause(NAME, true));
    test_int_success(service_continue(NAME, true));
    test_rc_success(service_status(NAME), SERVICE_STATUS_RUNNING, SERVICE_STATUS_INVALID);
}

TEST_CASE_PFIX(service_continue_running, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_error(service_continue(NAME, true), E_SERVICE_RUNNING);
}

TEST_CASE_ABORT(service_stop_invalid_name1)
{
    service_stop(NULL, true);
}

TEST_CASE(service_stop_invalid_name2)
{
    test_int_error(service_stop("", true), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_stop_unknown)
{
    test_int_error(service_stop("foobarbaz", true), E_SERVICE_UNKNOWN);
}

TEST_CASE_PFIX(service_stop_unsupported, service_install, service_kill_remove, NULL, NULL, "service nostop")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_error(service_stop(NAME, true), E_SERVICE_UNSUPPORTED);
}

TEST_CASE_PFIX(service_stop_stopped, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_error(service_stop(NAME, true), E_SERVICE_STOPPED);
}

TEST_CASE_PFIX(service_stop_paused, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_success(service_pause(NAME, true));
    test_int_success(service_stop(NAME, true));
    test_rc_success(service_status(NAME), SERVICE_STATUS_STOPPED, SERVICE_STATUS_INVALID);
}

TEST_CASE_PFIX(service_stop_running, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_success(service_stop(NAME, true));
    test_rc_success(service_status(NAME), SERVICE_STATUS_STOPPED, SERVICE_STATUS_INVALID);
}

TEST_CASE_ABORT(service_kill_invalid_name1)
{
    service_kill(NULL);
}

TEST_CASE(service_kill_invalid_name2)
{
    test_int_error(service_kill(""), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_kill_unknown)
{
    test_int_error(service_kill("foobarbaz"), E_SERVICE_UNKNOWN);
}

TEST_CASE_PFIX(service_kill, service_install, service_stop_remove, NULL, NULL, "service full")
{
    pid_t p1, p2;

    test_int_success(service_start(NAME, 0, NULL, true));
    test_int_success(p1 = service_pid(NAME));
    test_int_success(service_kill(NAME));
    sleep(1);
    test_int_success(p2 = service_pid(NAME));
    test_int_ne(p1, p2);
}

TEST_CASE_ABORT(service_disable_invalid_name1)
{
    service_disable(NULL);
}

TEST_CASE(service_disable_invalid_name2)
{
    test_int_error(service_disable(""), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_disable_unknown)
{
    test_int_error(service_disable("foobarbaz"), E_SERVICE_UNKNOWN);
}

TEST_CASE_PFIX(service_disable_disabled, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_disable(NAME));
    test_int_error(service_disable(NAME), E_SERVICE_DISABLED);
}

TEST_CASE_PFIX(service_disable_enabled, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_disable(NAME));
}

TEST_CASE_ABORT(service_enable_invalid_name1)
{
    service_enable(NULL, false);
}

TEST_CASE(service_enable_invalid_name2)
{
    test_int_error(service_enable("", false), E_SERVICE_INVALID_NAME);
}

TEST_CASE(service_enable_unknown)
{
    test_int_error(service_enable("foobarbaz", false), E_SERVICE_UNKNOWN);
}

TEST_CASE_PFIX(service_enable_disabled, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_success(service_disable(NAME));
    test_int_success(service_enable(NAME, false));
}

TEST_CASE_PFIX(service_enable_enabled, service_install, service_stop_remove, NULL, NULL, "service full")
{
    test_int_error(service_enable(NAME, false), E_SERVICE_ENABLED);
}

static const char *test_suite_sys_service_check(void)
{
    return IsUserAnAdmin() ? NULL : "Missing admin rights.";
}

int test_suite_sys_service(void *param)
{
    return error_pass_int(test_run_cases_check("service",
        test_suite_sys_service_check,

        test_case(service_install_invalid_name1),
        test_case(service_install_invalid_name2),
        test_case(service_install_invalid_user),
        test_case(service_install),
        test_case(service_install_system),
        test_case(service_install_current_exe),
        test_case(service_install_args),
        test_case(service_install_twice),
        test_case(service_remove_invalid_name1),
        test_case(service_remove_invalid_name2),
        test_case(service_remove_unknown),

        test_case(service_status_invalid_name1),
        test_case(service_status_invalid_name2),
        test_case(service_status_unknown),

        test_case(service_pid_invalid_name1),
        test_case(service_pid_invalid_name2),
        test_case(service_pid_unknown),
        test_case(service_pid),

        test_case(service_start_invalid_name1),
        test_case(service_start_invalid_name2),
        test_case(service_start_unknown),
        test_case(service_start_path_not_found),
        test_case(service_start_disabled),
        test_case(service_start_stopped),
        test_case(service_start_paused),
        test_case(service_start_running),

        test_case(service_pause_invalid_name1),
        test_case(service_pause_invalid_name2),
        test_case(service_pause_unknown),
        test_case(service_pause_unsupported),
        test_case(service_pause_stopped),
        test_case(service_pause_paused),
        test_case(service_pause_running),

        test_case(service_continue_invalid_name1),
        test_case(service_continue_invalid_name2),
        test_case(service_continue_unknown),
        test_case(service_continue_stopped),
        test_case(service_continue_paused),
        test_case(service_continue_running),

        test_case(service_stop_invalid_name1),
        test_case(service_stop_invalid_name2),
        test_case(service_stop_unknown),
        test_case(service_stop_unsupported),
        test_case(service_stop_stopped),
        test_case(service_stop_paused),
        test_case(service_stop_running),

        test_case(service_kill_invalid_name1),
        test_case(service_kill_invalid_name2),
        test_case(service_kill_unknown),
        test_case(service_kill),

        test_case(service_disable_invalid_name1),
        test_case(service_disable_invalid_name2),
        test_case(service_disable_unknown),
        test_case(service_disable_disabled),
        test_case(service_disable_enabled),

        test_case(service_enable_invalid_name1),
        test_case(service_enable_invalid_name2),
        test_case(service_enable_unknown),
        test_case(service_enable_disabled),
        test_case(service_enable_enabled),

        NULL
    ));
}

static bool running;

static int test_service_main(void *ctx)
{
    running = true;

    while(running)
        sleep(1);

    return 0;
}

static void test_service_stop(void *ctx)
{
    running = false;
}

static void test_service_pause(bool pause, void *ctx)
{

}

int test_service(int argc, char *argv[])
{
    service_control_st ctrl =
    {
        .main   = test_service_main,
        .stop   = test_service_stop,
        .pause  = test_service_pause
    };
    int arg;

    for(arg = 1; arg < argc; arg++)
    {
        if(!strcmp(argv[arg], "nopause"))
            ctrl.pause = NULL;

        if(!strcmp(argv[arg], "nostop"))
            ctrl.stop = NULL;

        if(!strcmp(argv[arg], "abort"))
            abort();
    }

    return service_dispatch(&ctrl);
}

#else // if OS_WINDOWS

typedef int dummy;

#endif // if OS_WINDOWS
