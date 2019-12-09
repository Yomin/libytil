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
#include <ytil/sys/env.h>
#include <ytil/sys/fs.h>
#include <ytil/sys/path.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
    
    fs_stat(NULL, FS_LINK_FOLLOW, &fst);
}

#ifndef _WIN32
TEST_CASE(fs_stat_invalid_path2)
{
    fs_stat_st fst;
    
    test_ptr_success(path = path_new(LIT("c:\\foo"), PATH_STYLE_WINDOWS));
    test_ptr_error(fs_stat(path, FS_LINK_FOLLOW, &fst), E_FS_INVALID_PATH);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE_ABORT(fs_stat_invalid_fst, path_new, path_free)
{
    fs_stat(path, FS_LINK_FOLLOW, NULL);
}

TEST_CASE(fs_stat_not_found)
{
    fs_stat_st fst;
    
    test_ptr_success(path = path_new(LIT("/ytil_test"), PATH_STYLE_NATIVE));
    test_ptr_error(fs_stat(path, FS_LINK_FOLLOW, &fst), E_FS_NOT_FOUND);
    path_free(path);
}

static void _test_fs_file_new(void *_test_case_ctx, path_ct *path, str_ct *str)
{
    FILE *fp;
    
    test_ptr_success(*path = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_void(env_free());
    
    test_ptr_success(path_append(*path, LIT("ytil_test_file"), PATH_STYLE_NATIVE));
    test_ptr_success(*str = path_get(*path, PATH_STYLE_NATIVE));
    
    test_ptr_success_errno(fp = fopen(str_c(*str), "wb+"));
    test_int_success_errno(fprintf(fp, "YTIL TEST\n"));
    test_int_success_errno(fclose(fp));
}

TEST_CASE(fs_stat_file)
{
    fs_stat_st fst;
    struct stat st;
    
    _test_fs_file_new(TEST_CTX, &path, &str);
    test_int_success_errno(stat(str_c(str), &st));
    
    test_ptr_success(fs_stat(path, FS_LINK_FOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
    test_int_eq(fst.size, st.st_size);
    test_int_eq(fst.uid, st.st_uid);
    test_int_eq(fst.gid, st.st_gid);
    test_int_eq(fst.atime, st.st_atime);
    test_int_eq(fst.mtime, st.st_mtime);
    test_int_eq(fst.ctime, st.st_ctime);
    
    test_int_success_errno(unlink(str_c(str)));
    str_unref(str);
    path_free(path);
}

#ifndef _WIN32
TEST_CASE(fs_stat_link)
{
    str_ct lstr;
    fs_stat_st fst;
    
    _test_fs_file_new(TEST_CTX, &path, &str);
    test_ptr_success(path_drop(path, 1));
    test_ptr_success(path_append(path, LIT("ytil_test_link"), PATH_STYLE_NATIVE));
    test_ptr_success(lstr = path_get(path, PATH_STYLE_NATIVE));
    test_int_maybe_errno(symlink(str_c(str), str_c(lstr)), EEXIST);
    
    test_ptr_success(fs_stat(path, FS_LINK_FOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
    test_ptr_success(fs_stat(path, FS_LINK_NOFOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_LINK);
    
    test_int_success_errno(unlink(str_c(lstr)));
    test_int_success_errno(unlink(str_c(str)));
    str_unref(lstr);
    str_unref(str);
    path_free(path);
}
#endif

TEST_CASE(fs_stat_dir)
{
    fs_stat_st fst;
    
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_void(env_free());
    
    test_ptr_success(path_append(path, LIT("ytil_test_dir"), PATH_STYLE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_NATIVE));
    test_int_maybe_errno(mkdir(str_c(str), S_IRWXU|S_IRWXG), EEXIST);
    
    test_ptr_success(fs_stat(path, FS_LINK_FOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
    
    test_int_success_errno(rmdir(str_c(str)));
    str_unref(str);
    path_free(path);
}

typedef struct fs_walk_test
{
    size_t dir, file, depth;
} fs_walk_test_st;

static int _test_fs_walk(path_const_ct file, size_t depth, fs_stat_st *info, void *ctx)
{
    fs_walk_test_st *test = ctx;
    
    switch(info->type)
    {
    case FS_TYPE_REGULAR:   test->file++; break;
    case FS_TYPE_DIRECTORY: test->dir++; break;
    default:                abort();
    }
    
    test->depth = depth;
    
    return 0;
}

TEST_CASE_ABORT(fs_walk_invalid_path1)
{
    fs_walk(NULL, 0, FS_WALK_PREORDER, FS_LINK_FOLLOW, _test_fs_walk, NULL);
}

#ifndef _WIN32
TEST_CASE(fs_walk_invalid_path2)
{
    test_ptr_success(path = path_new(LIT("c:\\foo"), PATH_STYLE_WINDOWS));
    test_int_error(fs_walk(path, 0, FS_WALK_PREORDER, FS_LINK_FOLLOW, _test_fs_walk, NULL), E_FS_INVALID_PATH);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE_ABORT(fs_walk_invalid_callback, path_new, path_free)
{
    fs_walk(path, 0, FS_WALK_PREORDER, FS_LINK_FOLLOW, NULL, NULL);
}

static void _test_fs_mkdir(void *_test_case_ctx, path_ct base, const char *dir, bool mkfile, size_t drop)
{
    FILE *fp;
    
    test_ptr_success(path_append_c(base, dir, PATH_STYLE_POSIX));
    test_ptr_success(str = path_get(base, PATH_STYLE_NATIVE));
    test_int_maybe_errno(mkdir(str_c(str), S_IRWXU|S_IRWXG), EEXIST);
    str_unref(str);
    
    if(mkfile)
    {
        test_ptr_success(path_append_c(base, "file", PATH_STYLE_POSIX));
        test_ptr_success(str = path_get(base, PATH_STYLE_NATIVE));
        test_ptr_maybe_errno(fp = fopen(str_c(str), "w"), EEXIST);
        test_int_success_errno(fclose(fp));
        str_unref(str);
    }
    
    test_ptr_success(path_drop(base, drop+(mkfile?1:0)));
}

TEST_SETUP(fs_mktree)
{
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_TMP));
    
    _test_fs_mkdir(TEST_CTX, path, "ytil_test_dir2", false, 0);
    _test_fs_mkdir(TEST_CTX, path, "foo", false, 0);
    _test_fs_mkdir(TEST_CTX, path, "foo1", true, 1);
    _test_fs_mkdir(TEST_CTX, path, "foo2", true, 1);
    _test_fs_mkdir(TEST_CTX, path, "foo3", true, 2);
    _test_fs_mkdir(TEST_CTX, path, "bar", false, 0);
    _test_fs_mkdir(TEST_CTX, path, "bar1", true, 1);
    _test_fs_mkdir(TEST_CTX, path, "bar2", true, 1);
    _test_fs_mkdir(TEST_CTX, path, "bar3", true, 2);
    _test_fs_mkdir(TEST_CTX, path, "baz", false, 0);
    _test_fs_mkdir(TEST_CTX, path, "baz1", true, 1);
    _test_fs_mkdir(TEST_CTX, path, "baz2", true, 1);
    _test_fs_mkdir(TEST_CTX, path, "baz3", true, 2);
}

static int _test_fs_rmdir(path_const_ct file, size_t depth, fs_stat_st *info, void *ctx)
{
    str_ct str = path_get(file, PATH_STYLE_NATIVE);
    
    switch(info->type)
    {
    case FS_TYPE_REGULAR:   unlink(str_c(str)); break;
    case FS_TYPE_DIRECTORY: rmdir(str_c(str)); break;
    default:                abort();
    }
    
    str_unref(str);
    
    return 0;
}

TEST_TEARDOWN(fs_rmtree)
{
    fs_walk(path, -1, FS_WALK_POSTORDER, FS_LINK_NOFOLLOW, _test_fs_rmdir, NULL);
    
    env_free();
    path_free(path);
}

TEST_CASE_FIXTURE(fs_walk_depth0, fs_mktree, fs_rmtree)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path, 0, FS_WALK_PREORDER, FS_LINK_FOLLOW, _test_fs_walk, &test));
    test_uint_eq(test.dir, 1);
    test_uint_eq(test.file, 0);
}

TEST_CASE_FIXTURE(fs_walk_depth1, fs_mktree, fs_rmtree)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path, 1, FS_WALK_PREORDER, FS_LINK_FOLLOW, _test_fs_walk, &test));
    test_uint_eq(test.dir, 4);
    test_uint_eq(test.file, 0);
}

TEST_CASE_FIXTURE(fs_walk_recursive_preorder, fs_mktree, fs_rmtree)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path, -1, FS_WALK_PREORDER, FS_LINK_FOLLOW, _test_fs_walk, &test));
    test_uint_eq(test.dir, 13);
    test_uint_eq(test.file, 9);
    test_uint_eq(test.depth, 3);
}

TEST_CASE_FIXTURE(fs_walk_recursive_postorder, fs_mktree, fs_rmtree)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path, -1, FS_WALK_POSTORDER, FS_LINK_FOLLOW, _test_fs_walk, &test));
    test_uint_eq(test.dir, 13);
    test_uint_eq(test.file, 9);
    test_uint_eq(test.depth, 0);
}

test_suite_ct test_suite_sys_fsys(void)
{
    return test_suite_new_with_cases("fs"
        , test_case_new(fs_stat_invalid_path1)
        , test_case_new_unix(fs_stat_invalid_path2)
        , test_case_new(fs_stat_invalid_fst)
        , test_case_new(fs_stat_not_found)
        , test_case_new(fs_stat_file)
        , test_case_new_unix(fs_stat_link)
        , test_case_new(fs_stat_dir)
        
        , test_case_new(fs_walk_invalid_path1)
        , test_case_new_unix(fs_walk_invalid_path2)
        , test_case_new(fs_walk_invalid_callback)
        , test_case_new(fs_walk_depth0)
        , test_case_new(fs_walk_depth1)
        , test_case_new(fs_walk_recursive_preorder)
        , test_case_new(fs_walk_recursive_postorder)
    );
}
