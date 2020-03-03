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

#include "fs.h"
#include <ytil/test/test.h>
#include <ytil/sys/env.h>
#include <ytil/sys/fs.h>
#include <ytil/sys/path.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef _WIN32
#   define mkdir(path, mode) mkdir(path)
#else
#   define mkdir(path, mode) mkdir(path, mode)
#endif

typedef enum mkfile_type
{
      MKNONE
    , MKPATH
    , MKDRIVE
    , MKFILE
    , MKLINK
    , MKDIR
    , MKTREE
    , MKTREE_BLOCKER
} mkfile_type_id;

static mkfile_type_id type1, type2;
static path_ct path1, path2;
static str_ct str1, str2;
static fs_stat_st fst;


TEST_FUNCTION(void, mkdir, path_ct base, const char *dir, int mode, bool mkfile, size_t drop)
{
    str_ct str;
    FILE *fp;
    
    test_ptr_success(path_append_c(base, dir, PATH_STYLE_POSIX));
    test_ptr_success(str = path_get(base, PATH_STYLE_NATIVE));
    test_int_maybe_errno(mkdir(str_c(str), mode), EEXIST);
    test_int_success_errno(chmod(str_c(str), mode));
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

TEST_FUNCTION(void, mktree, mkfile_type_id type, str_ct name, path_ct *path)
{
    test_ptr_success(*path = path_get_base_dir(PATH_BASE_DIR_TMP));
    
    test_call(mkdir, *path, str_c(name), S_IRWXU, false, 0);
    test_call(mkdir, *path, "foo", S_IRWXU, false, 0);
    test_call(mkdir, *path, "foo1", S_IRWXU, true, 1);
    test_call(mkdir, *path, "foo2", S_IRWXU, true, 1);
    test_call(mkdir, *path, "foo3", S_IRWXU, true, 2);
    
    if(type == MKTREE_BLOCKER)
        test_call(mkdir, *path, "bar", 0, false, 1);
    else
    {
        test_call(mkdir, *path, "bar", S_IRWXU, false, 0);
        test_call(mkdir, *path, "bar1", S_IRWXU, true, 1);
        test_call(mkdir, *path, "bar2", S_IRWXU, true, 1);
        test_call(mkdir, *path, "bar3", S_IRWXU, true, 2);
    }
    
    test_call(mkdir, *path, "baz", S_IRWXU, false, 0);
    test_call(mkdir, *path, "baz1", S_IRWXU, true, 1);
    test_call(mkdir, *path, "baz2", S_IRWXU, true, 1);
    test_call(mkdir, *path, "baz3", S_IRWXU, true, 2);
}

static int _test_rmfile_error(fs_walk_type_id type, path_const_ct file, size_t depth, fs_stat_st *info, void *ctx)
{
    void *_test_case_ctx = ctx;
    str_ct str;
    
    if(!error_check(0, E_FS_NOT_FOUND) && (str = path_get(file, PATH_STYLE_NATIVE)))
    {
        if(!chmod(str_c(str), S_IRWXU) && !rmdir(str_c(str)))
            return str_unref(str), 0;
        
        test_msg_backtrace("failed to remove '%s'", str_c(str));
        str_unref(str);
    }
    
    return error_pass(), -1;
}

TEST_FUNCTION(void, mkfile, mkfile_type_id type, str_ct name, path_ct *path, str_ct *str, path_ct other_path, str_ct other_str)
{
    FILE *fp;
    
    switch(type)
    {
    case MKNONE:
        *path = NULL;
        *str = NULL;
        break;
    case MKPATH:
    case MKFILE:
    case MKLINK:
    case MKDIR:
        test_ptr_success(*path = path_get_base_dir(PATH_BASE_DIR_TMP));
        test_ptr_success(path_append(*path, name, PATH_STYLE_NATIVE));
        test_ptr_success(*str = path_get(*path, PATH_STYLE_NATIVE));
        test_int_lift_maybe(fs_remove_f(*path, _test_rmfile_error, TEST_CTX), E_FS_CALLBACK, E_FS_NOT_FOUND);
        break;
    case MKDRIVE:
        test_ptr_success(*path = path_new(LIT("c:\\"), PATH_STYLE_WINDOWS));
        test_ptr_success(path_append(*path, name, PATH_STYLE_POSIX));
        break;
    case MKTREE:
    case MKTREE_BLOCKER:
        test_call(mktree, type, name, path);
        *str = NULL;
        break;
    default:
        break;
    }
    
    switch(type)
    {
    case MKFILE:
        test_ptr_success_errno(fp = fopen(str_c(*str), "wb+"));
        test_int_success_errno(fprintf(fp, "YTIL TEST\n"));
        test_int_success_errno(fclose(fp));
        break;
    case MKLINK:
#ifndef _WIN32
        test_int_success_errno(symlink(str_c(other_str), str_c(*str)));
#else
        test_abort("symlink unavailable");
#endif
        break;
    case MKDIR:
        test_int_success_errno(mkdir(str_c(*str), S_IRWXU));
        break;
    default:
        break;
    }
}

TEST_SETUP(mkfile, mkfile_type_id t1, mkfile_type_id t2)
{
    type1 = t1;
    type2 = t2;
    test_call(mkfile, type1, LIT("ytil_test1"), &path1, &str1, NULL, NULL);
    test_call(mkfile, type2, LIT("ytil_test2"), &path2, &str2, path1, str1);
    test_void(env_free());
}

TEST_FUNCTION(void, rmfile, mkfile_type_id type, path_ct path, str_ct str)
{
    switch(type)
    {
    case MKDRIVE:
        path_free(path);
        break;
    case MKPATH:
    case MKFILE:
    case MKLINK:
    case MKDIR:
        test_int_lift_maybe(fs_remove_f(path, _test_rmfile_error, TEST_CTX), E_FS_CALLBACK, E_FS_NOT_FOUND);
        path_free(path);
        str_unref(str);
        break;
    case MKTREE:
    case MKTREE_BLOCKER:
        test_int_lift_maybe(fs_remove_f(path, _test_rmfile_error, TEST_CTX), E_FS_CALLBACK, E_FS_NOT_FOUND);
        path_free(path);
        break;
    default:
        break;
    }
}

TEST_TEARDOWN(rmfile)
{
    test_call(rmfile, type1, path1, str1);
    test_call(rmfile, type2, path2, str2);
}

TEST_FUNCTION(void, ckfile)
{

}

TEST_CASE_ABORT(fs_stat_invalid_path1)
{
    fs_stat(NULL, FS_STAT_DEFAULT, &fst);
}

#ifndef _WIN32
TEST_CASE_ARGS(fs_stat_invalid_path2, mkfile, rmfile, MKDRIVE, MKNONE)
{
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_ABORT_ARGS(fs_stat_invalid_fst, mkfile, rmfile, MKPATH, MKNONE)
{
    fs_stat(path1, FS_STAT_DEFAULT, NULL);
}

TEST_CASE_ARGS(fs_stat_not_found, mkfile, rmfile, MKPATH, MKNONE)
{
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
}

TEST_CASE_ARGS(fs_stat_file, mkfile, rmfile, MKFILE, MKNONE)
{
    struct stat st;
    
    test_int_success_errno(stat(str_c(str1), &st));
    
    test_ptr_success(fs_stat(path1, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
    test_int_eq(fst.size, st.st_size);
    test_int_eq(fst.uid, st.st_uid);
    test_int_eq(fst.gid, st.st_gid);
    test_int_eq(fst.atime, st.st_atime);
    test_int_eq(fst.mtime, st.st_mtime);
    test_int_eq(fst.ctime, st.st_ctime);
}

#ifndef _WIN32
TEST_CASE_ARGS(fs_stat_link, mkfile, rmfile, MKFILE, MKLINK)
{
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
    test_ptr_success(fs_stat(path2, FS_STAT_LINK_NOFOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_LINK);
}
#endif

TEST_CASE_ARGS(fs_stat_dir, mkfile, rmfile, MKDIR, MKNONE)
{
    test_ptr_success(fs_stat(path1, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
}

typedef struct fs_walk_test
{
    size_t dir_pre, dir_post, file;
} fs_walk_test_st;

static int _test_fs_walk(fs_walk_type_id type, path_const_ct file, size_t depth, fs_stat_st *info, void *ctx)
{
    fs_walk_test_st *test = ctx;
    
    switch(type)
    {
    case FS_WALK_FILE:          test->file++; break;
    case FS_WALK_DIR_PRE:       test->dir_pre++; break;
    case FS_WALK_DIR_POST:      test->dir_post++; break;
    case FS_WALK_DIR_ERROR:
    case FS_WALK_STAT_ERROR:    error_pass(); return -1;
    default:                    abort();
    }
    
    return 0;
}

TEST_CASE_ABORT(fs_walk_invalid_path1)
{
    fs_walk(NULL, 0, FS_STAT_DEFAULT, _test_fs_walk, NULL);
}

#ifndef _WIN32
TEST_CASE_ARGS(fs_walk_invalid_path2, mkfile, rmfile, MKDRIVE, MKNONE)
{
    test_int_error(fs_walk(path1, 0, FS_STAT_DEFAULT, _test_fs_walk, NULL), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_ABORT_ARGS(fs_walk_invalid_callback, mkfile, rmfile, MKPATH, MKNONE)
{
    fs_walk(path1, 0, FS_STAT_DEFAULT, NULL, NULL);
}

TEST_CASE_ARGS(fs_walk_not_found, mkfile, rmfile, MKPATH, MKNONE)
{
    fs_walk_test_st test = {0};
    
    test_int_lift_error(fs_walk(path1, 0, FS_STAT_DEFAULT, _test_fs_walk, &test), E_FS_CALLBACK, E_FS_NOT_FOUND);
}

TEST_CASE_ARGS(fs_walk_depth0, mkfile, rmfile, MKTREE, MKNONE)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path1, 0, FS_STAT_DEFAULT, _test_fs_walk, &test));
    test_uint_eq(test.dir_pre, 1);
    test_uint_eq(test.dir_post, 1);
    test_uint_eq(test.file, 0);
}

TEST_CASE_ARGS(fs_walk_depth1, mkfile, rmfile, MKTREE, MKNONE)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path1, 1, FS_STAT_DEFAULT, _test_fs_walk, &test));
    test_uint_eq(test.dir_pre, 4);
    test_uint_eq(test.dir_post, 4);
    test_uint_eq(test.file, 0);
}

TEST_CASE_ARGS(fs_walk_recursive, mkfile, rmfile, MKTREE, MKNONE)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path1, -1, FS_STAT_DEFAULT, _test_fs_walk, &test));
    test_uint_eq(test.dir_pre, 13);
    test_uint_eq(test.dir_post, 13);
    test_uint_eq(test.file, 9);
}

TEST_CASE_ABORT(fs_remove_f_invalid_path1)
{
    fs_remove_f(NULL, NULL, NULL);
}

#ifndef _WIN32
TEST_CASE_ARGS(fs_remove_f_invalid_path2, mkfile, rmfile, MKDRIVE, MKNONE)
{
    test_int_error(fs_remove_f(path1, NULL, NULL), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_ARGS(fs_remove_f_not_found, mkfile, rmfile, MKPATH, MKNONE)
{
    test_int_error(fs_remove_f(path1, NULL, NULL), E_FS_NOT_FOUND);
}

TEST_CASE_ARGS(fs_remove_f_file, mkfile, rmfile, MKFILE, MKNONE)
{
    test_int_success(fs_remove_f(path1, NULL, NULL));
    test_ptr_error(fs_stat(path1, FS_STAT_LINK_NOFOLLOW, &fst), E_FS_NOT_FOUND);
}

TEST_CASE_ARGS(fs_remove_f_dir, mkfile, rmfile, MKDIR, MKNONE)
{
    test_int_success(fs_remove_f(path1, NULL, NULL));
    test_ptr_error(fs_stat(path1, FS_STAT_LINK_NOFOLLOW, &fst), E_FS_NOT_FOUND);
}

TEST_CASE_ARGS(fs_remove_f_tree, mkfile, rmfile, MKTREE, MKNONE)
{
    test_int_success(fs_remove_f(path1, NULL, NULL));
    test_ptr_error(fs_stat(path1, FS_STAT_LINK_NOFOLLOW, &fst), E_FS_NOT_FOUND);
}

static int _test_fs_remove_error(fs_walk_type_id type, path_const_ct file, size_t depth, fs_stat_st *info, void *ctx)
{
    str_ct *blocker = ctx;
    
    *blocker = path_basename(file, PATH_STYLE_NATIVE);
    
    return error_pass(), -1;
}

TEST_CASE_ARGS(fs_remove_f_tree_fail, mkfile, rmfile, MKTREE_BLOCKER, MKNONE)
{
    str_ct blocker;
    
    test_int_lift_error(fs_remove_f(path1, _test_fs_remove_error, &blocker), E_FS_CALLBACK, E_FS_ACCESS_DENIED);
    
    test_str_eq(str_c(blocker), "bar");
    str_unref(blocker);
}

TEST_CASE_ABORT_ARGS(fs_move_invalid_src1, mkfile, rmfile, MKNONE, MKPATH)
{
    fs_move(NULL, path2, FS_MOVE_DEFAULT);
}

#ifndef _WIN32
TEST_CASE_ARGS(fs_move_invalid_src2, mkfile, rmfile, MKDRIVE, MKPATH)
{
    test_int_error(fs_move(path1, path2, FS_MOVE_DEFAULT), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_ABORT_ARGS(fs_move_invalid_dst1, mkfile, rmfile, MKPATH, MKNONE)
{
    fs_move(path1, NULL, FS_MOVE_DEFAULT);
}

#ifndef _WIN32
TEST_CASE_ARGS(fs_move_invalid_dst2, mkfile, rmfile, MKPATH, MKDRIVE)
{
    test_int_error(fs_move(path1, path2, FS_MOVE_DEFAULT), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_ARGS(fs_move_not_found, mkfile, rmfile, MKPATH, MKPATH)
{
    test_int_error(fs_move(path1, path2, FS_MOVE_DEFAULT), E_FS_NOT_FOUND);
}

TEST_CASE_ARGS(fs_move_file, mkfile, rmfile, MKFILE, MKPATH)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    
    test_call(ckfile);
    
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
}

TEST_CASE_ARGS(fs_move_file_replace_file, mkfile, rmfile, MKFILE, MKFILE)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
}

TEST_CASE_ARGS(fs_move_file_replace_dir, mkfile, rmfile, MKFILE, MKDIR)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
}

TEST_CASE_ARGS(fs_move_file_replace_tree, mkfile, rmfile, MKFILE, MKTREE)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
}

TEST_CASE_ARGS(fs_move_dir, mkfile, rmfile, MKDIR, MKPATH)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
}

TEST_CASE_ARGS(fs_move_dir_replace_file, mkfile, rmfile, MKDIR, MKFILE)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
}

TEST_CASE_ARGS(fs_move_dir_replace_dir, mkfile, rmfile, MKDIR, MKDIR)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
}

TEST_CASE_ARGS(fs_move_dir_replace_tree, mkfile, rmfile, MKDIR, MKTREE)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
}

TEST_CASE_ARGS(fs_move_tree, mkfile, rmfile, MKTREE, MKPATH)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
}

TEST_CASE_ARGS(fs_move_tree_replace_file, mkfile, rmfile, MKTREE, MKFILE)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
}

TEST_CASE_ARGS(fs_move_tree_replace_dir, mkfile, rmfile, MKTREE, MKDIR)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
}

TEST_CASE_ARGS(fs_move_tree_replace_tree, mkfile, rmfile, MKTREE, MKTREE)
{
    test_int_success(fs_move(path1, path2, FS_MOVE_DEFAULT));
    test_ptr_error(fs_stat(path1, FS_STAT_DEFAULT, &fst), E_FS_NOT_FOUND);
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
}

TEST_CASE_ABORT_ARGS(fs_copy_invalid_src1, mkfile, rmfile, MKNONE, MKPATH)
{
    fs_copy(NULL, path2, FS_COPY_DEFAULT);
}

#ifndef _WIN32
TEST_CASE_ARGS(fs_copy_invalid_src2, mkfile, rmfile, MKDRIVE, MKPATH)
{
    test_int_error(fs_copy(path1, path2, FS_COPY_DEFAULT), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_ABORT_ARGS(fs_copy_invalid_dst1, mkfile, rmfile, MKPATH, MKNONE)
{
    fs_copy(path1, NULL, FS_COPY_DEFAULT);
}

#ifndef _WIN32
TEST_CASE_ARGS(fs_copy_invalid_dst2, mkfile, rmfile, MKPATH, MKDRIVE)
{
    test_int_error(fs_copy(path1, path2, FS_COPY_DEFAULT), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_ARGS(fs_copy_not_found, mkfile, rmfile, MKPATH, MKPATH)
{
    test_int_error(fs_copy(path1, path2, FS_COPY_DEFAULT), E_FS_NOT_FOUND);
}

TEST_CASE_ARGS(fs_copy_file, mkfile, rmfile, MKFILE, MKPATH)
{
    test_int_success(fs_copy(path1, path2, FS_COPY_DEFAULT));
    test_ptr_success(fs_stat(path1, FS_STAT_DEFAULT, &fst));
    test_ptr_success(fs_stat(path2, FS_STAT_DEFAULT, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
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
        , test_case_new(fs_walk_not_found)
        , test_case_new(fs_walk_depth0)
        , test_case_new(fs_walk_depth1)
        , test_case_new(fs_walk_recursive)
        
        , test_case_new(fs_remove_f_invalid_path1)
        , test_case_new_unix(fs_remove_f_invalid_path2)
        , test_case_new(fs_remove_f_not_found)
        , test_case_new(fs_remove_f_file)
        , test_case_new(fs_remove_f_dir)
        , test_case_new(fs_remove_f_tree)
        , test_case_new(fs_remove_f_tree_fail)
        
        , test_case_new(fs_move_invalid_src1)
        , test_case_new_unix(fs_move_invalid_src2)
        , test_case_new(fs_move_invalid_dst1)
        , test_case_new_unix(fs_move_invalid_dst2)
        , test_case_new(fs_move_not_found)
        , test_case_new(fs_move_file)
        , test_case_new(fs_move_file_replace_file)
        , test_case_new(fs_move_file_replace_dir)
        , test_case_new(fs_move_file_replace_tree)
        , test_case_new(fs_move_dir)
        , test_case_new(fs_move_dir_replace_file)
        , test_case_new(fs_move_dir_replace_dir)
        , test_case_new(fs_move_dir_replace_tree)
        , test_case_new(fs_move_tree)
        , test_case_new(fs_move_tree_replace_file)
        , test_case_new(fs_move_tree_replace_dir)
        , test_case_new(fs_move_tree_replace_tree)
        
        , test_case_new(fs_copy_invalid_src1)
        , test_case_new_unix(fs_copy_invalid_src2)
        , test_case_new(fs_copy_invalid_dst1)
        , test_case_new_unix(fs_copy_invalid_dst2)
        , test_case_new(fs_copy_not_found)
        , test_case_new(fs_copy_file)
    );
}
