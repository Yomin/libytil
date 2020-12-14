/*
 * Copyright (c) 2019-2020 Martin Rödel a.k.a. Yomin Nimoy
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

#include <ytil/test/run.h>
#include "con/cont.h"
#include "enc/enc.h"
#include "gen/gen.h"
#include "sys/sys.h"
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
    int rc;

#if _WIN32

    if(argc >= 2 && !strcmp(argv[1], "service"))
        return test_service(argc - 1, argv + 1);

#endif

    if(test_run_init_from_args(argc, argv))
    {
        if(error_code(0) == E_TEST_USAGE)
            test_run_print_usage(argv[0]);
        else
            fprintf(stderr, "failed to create test run: %s\n", error_desc(0));

        return -1;
    }

    rc = test_run_suites(NULL,
        test_suite(con),
        test_suite(enc),
        test_suite(gen),
        test_suite(sys),
        NULL
    );

    if(rc > 0) // worker process
    {
        rc = 0;
    }
    else if(!rc || error_code(0) == E_TEST_STOP)
    {
        test_run_print_summary();
        rc = 0;
    }
    else
    {
        fprintf(stderr, "failed to run test suites: %s\n", error_desc(0));
    }

    test_run_free();

    return rc;
}
