/*
 * Copyright (c) 2019 Martin Rödel a.k.a. Yomin Nimoy
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

#include "fs.h"
#include <ytil/test/test.h>
#include <ytil/sys/fs.h>
#include <ytil/sys/path.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

static path_ct path;
static str_ct str;


TEST_SETUP(path_new)
{
    test_ptr_success(path = path_new_current());
}

TEST_TEARDOWN(path_free)
{
    path_free(path);
}

TEST_CASE_ABORT(fs_stat_invalid_path1)
{
    fs_stat_st fst;
    
    fs_stat(NULL, FS_FLAG_FOLLOW, &fst);
}

#ifndef _WIN32
TEST_CASE(fs_stat_invalid_path2)
{
    fs_stat_st fst;
    
    test_ptr_success(path = path_new(LIT("c:\\foo"), PATH_STYLE_WINDOWS));
    test_ptr_error(fs_stat(path, FS_FLAG_FOLLOW, &fst), E_FS_INVALID_PATH);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE_ABORT(fs_stat_invalid_fst, path_new, path_free)
{
    fs_stat(path, FS_FLAG_FOLLOW, NULL);
}

TEST_CASE(fs_stat_not_found)
{
    fs_stat_st fst;
    
    test_ptr_success(path = path_new(LIT("/ytil_test"), PATH_STYLE_NATIVE));
    test_ptr_error(fs_stat(path, FS_FLAG_FOLLOW, &fst), E_FS_NOT_FOUND);
    path_free(path);
}

TEST_CASE(fs_stat)
{
    FILE *fp;
    fs_stat_st fst;
    struct stat st;
    
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_ptr_success(path_append(path, LIT("ytil_test_file"), PATH_STYLE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_NATIVE));
    
    test_ptr_success_errno(fp = fopen(str_c(str), "wb+"));
    test_int_success_errno(fprintf(fp, "YTIL TEST\n"));
    test_int_success_errno(fclose(fp));
    test_int_success_errno(stat(str_c(str), &st));
    
    test_ptr_success(fs_stat(path, FS_FLAG_FOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
    test_int_eq(fst.size, st.st_size);
    test_int_eq(fst.uid, st.st_uid);
    test_int_eq(fst.gid, st.st_gid);
    
    unlink(str_c(str));
    str_unref(str);
    path_free(path);
}

test_suite_ct test_suite_sys_fsys(void)
{
    return test_suite_new_with_cases("fs"
        , test_case_new(fs_stat_invalid_path1)
        , test_case_new_unix(fs_stat_invalid_path2)
        , test_case_new(fs_stat_invalid_fst)
        , test_case_new(fs_stat_not_found)
        , test_case_new(fs_stat)
    );
}
