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

#include "gen.h"
#include <ytil/test/run.h>
#include <ytil/test/test.h>
//#include <ytil/gen/box.h>


/*static const struct not_a_box
{
    int foo;
} not_a_box = { 123 };

static box_ct box;


TEST_CASE(box_new)
{
    test_ptr_success(box = box_new(TYPE_INT, NULL));
}

TEST_CASE_ABORT(box_free_invalid_magic)
{
    box_free((box_ct)&not_a_box);
}*/

int test_suite_gen_box(void *param)
{
    /*return error_pass_int(test_run_cases("box",
        test_case(box_new),
        test_case(box_free_invalid_magic),

        NULL
    );*/

    return 0;
}
