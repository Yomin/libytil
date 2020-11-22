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


int main(int argc, char *argv[])
{
    test_suite_ct suite;
    test_run_ct run;
    int rc = -1;

    suite = test_suite_new_with_suites("ytil",
        test_suite_con(),
        test_suite_enc(),
        test_suite_gen(),
        test_suite_sys()
    );

    if(!suite)
        return fprintf(stderr, "failed to setup test suites: %s\n", error_desc(0)), -1;

    if((run = test_run_new_with_args(argc, argv)))
    {
        rc = test_run_exec(run, suite);
        test_run_free(run);
    }
    else
    {
        fprintf(stderr, "failed to run test suites: %s\n", error_desc(0));
    }

    test_suite_free(suite);

    return rc;
}