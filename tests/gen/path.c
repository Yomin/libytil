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

#include "path.h"
#include <ytil/test/test.h>
#include <ytil/gen/path.h>

static struct not_a_path
{
    int foo;
} not_a_path = { 123 };

path_ct path;
str_ct str;
str_const_ct cstr;

TEST_TEARDOWN(path_free)
{
    path_free(path);
}

TEST_CASE_SIGNAL(path_new_invalid_str, SIGABRT)
{
    path_new((str_ct)&not_a_path, PATH_STYLE_SYSTEM);
}

TEST_CASE_SIGNAL(path_new_invalid_style, SIGABRT)
{
    path_new(LIT("foo"), 999);
}

TEST_CASE(path_new_empty)
{
    test_ptr_error(path_new(LIT(""), PATH_STYLE_SYSTEM), E_PATH_MALFORMED);
}

TEST_SETUP(path_new_root)
{
    test_ptr_success(path = path_new(LIT("/"), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_root, path_new_root, path_free)
{
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/");
    str_unref(str);
}

TEST_CASE(path_new_posix_root2)
{
    test_ptr_error(path_new(LIT("//"), PATH_STYLE_POSIX), E_PATH_UNSUPPORTED);
}

TEST_CASE(path_new_posix_root3)
{
    test_ptr_success(path = path_new(LIT("///"), PATH_STYLE_POSIX));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/");
    path_free(path);
    str_unref(str);
}

TEST_SETUP(path_new_current)
{
    test_ptr_success(path = path_new(LIT("."), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_current, path_new_current, path_free)
{
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("."));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), ".");
    str_unref(str);
}

TEST_CASE(path_new_posix_current_dir)
{
    test_ptr_success(path = path_new(LIT("./"), PATH_STYLE_POSIX));
    test_true(path_is_relative(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("./"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "./");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_posix_current_absolute)
{
    test_ptr_success(path = path_new(LIT("/."), PATH_STYLE_POSIX));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_posix_current_absolute_dir)
{
    test_ptr_success(path = path_new(LIT("/./"), PATH_STYLE_POSIX));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/");
    path_free(path);
    str_unref(str);
}

TEST_SETUP(path_new_parent)
{
    test_ptr_success(path = path_new(LIT(".."), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_parent, path_new_parent, path_free)
{
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen(".."));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "..");
    str_unref(str);
}

TEST_CASE(path_new_posix_parent_dir)
{
    test_ptr_success(path = path_new(LIT("../"), PATH_STYLE_POSIX));
    test_true(path_is_relative(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("../"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "../");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_posix_parent_absolute)
{
    test_ptr_success(path = path_new(LIT("/.."), PATH_STYLE_POSIX));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/.."));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/..");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_posix_parent_absolute_dir)
{
    test_ptr_success(path = path_new(LIT("/../"), PATH_STYLE_POSIX));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/../"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/../");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_posix_parent_current)
{
    test_ptr_success(path = path_new(LIT("../."), PATH_STYLE_POSIX));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen(".."));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "..");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_posix_parent_current_dir)
{
    test_ptr_success(path = path_new(LIT(".././"), PATH_STYLE_POSIX));
    test_true(path_is_relative(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("../"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "../");
    path_free(path);
    str_unref(str);
}

TEST_SETUP(path_new_file_relative)
{
    test_ptr_success(path = path_new(LIT("foo.txt"), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_file_relative, path_new_file_relative, path_free)
{
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("foo.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "foo.txt");
    str_unref(str);
}

TEST_SETUP(path_new_file_absolute)
{
    test_ptr_success(path = path_new(LIT("/foo.txt"), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_file_absolute, path_new_file_absolute, path_free)
{
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/foo.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo.txt");
    str_unref(str);
}

TEST_SETUP(path_new_path_relative)
{
    test_ptr_success(path = path_new(LIT("foo/bar/../.baz.boz"), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_path_relative, path_new_path_relative, path_free)
{
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("foo/bar/../.baz.boz"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "foo/bar/../.baz.boz");
    str_unref(str);
}

TEST_SETUP(path_new_path_absolute)
{
    test_ptr_success(path = path_new(LIT("/foo/bar/../.baz.boz"), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_path_absolute, path_new_path_absolute, path_free)
{
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/foo/bar/../.baz.boz"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz");
    str_unref(str);
}

TEST_CASE(path_new_posix_current_file)
{
    test_ptr_success(path = path_new(LIT("./foo.txt"), PATH_STYLE_POSIX));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("foo.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "foo.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_posix_current_path)
{
    test_ptr_success(path = path_new(LIT("./foo/bar/../.baz.boz"), PATH_STYLE_POSIX));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("foo/bar/../.baz.boz"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "foo/bar/../.baz.boz");
    path_free(path);
    str_unref(str);
}

TEST_SETUP(path_new_dir_relative)
{
    test_ptr_success(path = path_new(LIT("foo/bar/../.baz.boz/"), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_dir_relative, path_new_dir_relative, path_free)
{
    test_true(path_is_relative(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("foo/bar/../.baz.boz/"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "foo/bar/../.baz.boz/");
    str_unref(str);
}

TEST_SETUP(path_new_dir_absolute)
{
    test_ptr_success(path = path_new(LIT("/foo/bar/../.baz.boz/"), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_dir_absolute, path_new_dir_absolute, path_free)
{
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/foo/bar/../.baz.boz/"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/");
    str_unref(str);
}

TEST_SETUP(path_new_dir_current_relative)
{
    test_ptr_success(path = path_new(LIT("foo/bar/../.baz.boz/."), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_dir_current_relative, path_new_dir_current_relative, path_free)
{
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 5);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("foo/bar/../.baz.boz/."));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "foo/bar/../.baz.boz/.");
    str_unref(str);
}

TEST_SETUP(path_new_dir_current_absolute)
{
    test_ptr_success(path = path_new(LIT("/foo/bar/../.baz.boz/."), PATH_STYLE_POSIX));
}

TEST_CASE_FIXTURE(path_new_posix_dir_current_absolute, path_new_dir_current_absolute, path_free)
{
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 5);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/foo/bar/../.baz.boz/."));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/.");
    str_unref(str);
}

TEST_CASE(path_new_posix_squash_current)
{
    test_ptr_success(path = path_new(LIT("./foo/./bar/././baz.txt"), PATH_STYLE_POSIX));
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("foo/bar/baz.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "foo/bar/baz.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_posix_squash_separator)
{
    test_ptr_success(path = path_new(LIT("/foo//bar///baz.txt"), PATH_STYLE_POSIX));
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), strlen("/foo/bar/baz.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/baz.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_root)
{
    test_ptr_success(path = path_new(LIT("\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_root2)
{
    test_ptr_error(path_new(LIT("\\\\"), PATH_STYLE_WINDOWS), E_PATH_MALFORMED);
}

TEST_CASE(path_new_windows_root3)
{
    test_ptr_success(path = path_new(LIT("\\\\\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_current)
{
    test_ptr_success(path = path_new(LIT("."), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), ".");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_current_dir)
{
    test_ptr_success(path = path_new(LIT(".\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen(".\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), ".\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_current_absolute)
{
    test_ptr_success(path = path_new(LIT("\\."), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_current_absolute_dir)
{
    test_ptr_success(path = path_new(LIT("\\.\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_parent)
{
    test_ptr_success(path = path_new(LIT(".."), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen(".."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "..");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_parent_dir)
{
    test_ptr_success(path = path_new(LIT("..\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("..\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "..\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_parent_absolute)
{
    test_ptr_success(path = path_new(LIT("\\.."), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\.."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\..");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_parent_absolute_dir)
{
    test_ptr_success(path = path_new(LIT("\\..\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\..\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\..\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_parent_current)
{
    test_ptr_success(path = path_new(LIT("..\\."), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen(".."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "..");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_parent_current_dir)
{
    test_ptr_success(path = path_new(LIT("..\\.\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("..\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "..\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_file_relative)
{
    test_ptr_success(path = path_new(LIT("foo.txt"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("foo.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "foo.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_file_absolute)
{
    test_ptr_success(path = path_new(LIT("\\foo.txt"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\foo.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\foo.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_path_relative)
{
    test_ptr_success(path = path_new(LIT("foo\\bar\\..\\.baz.boz"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("foo\\bar\\..\\.baz.boz"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "foo\\bar\\..\\.baz.boz");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_path_absolute)
{
    test_ptr_success(path = path_new(LIT("\\foo\\bar\\..\\.baz.boz"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\foo\\bar\\..\\.baz.boz"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\foo\\bar\\..\\.baz.boz");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_current_file)
{
    test_ptr_success(path = path_new(LIT(".\\foo.txt"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("foo.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "foo.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_current_path)
{
    test_ptr_success(path = path_new(LIT(".\\foo\\bar\\..\\.baz.boz"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("foo\\bar\\..\\.baz.boz"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "foo\\bar\\..\\.baz.boz");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_dir_relative)
{
    test_ptr_success(path = path_new(LIT("foo\\bar\\..\\.baz.boz\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("foo\\bar\\..\\.baz.boz\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "foo\\bar\\..\\.baz.boz\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_dir_absolute)
{
    test_ptr_success(path = path_new(LIT("\\foo\\bar\\..\\.baz.boz\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\foo\\bar\\..\\.baz.boz\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\foo\\bar\\..\\.baz.boz\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_dir_current_relative)
{
    test_ptr_success(path = path_new(LIT("foo\\bar\\..\\.baz.boz\\."), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 5);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("foo\\bar\\..\\.baz.boz\\."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "foo\\bar\\..\\.baz.boz\\.");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_dir_current_absolute)
{
    test_ptr_success(path = path_new(LIT("\\foo\\bar\\..\\.baz.boz\\."), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_STANDARD);
    test_uint_eq(path_depth(path), 5);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\foo\\bar\\..\\.baz.boz\\."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\foo\\bar\\..\\.baz.boz\\.");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_squash_current)
{
    test_ptr_success(path = path_new(LIT(".\\foo\\.\\bar\\.\\.\\baz.txt"), PATH_STYLE_WINDOWS));
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("foo\\bar\\baz.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "foo\\bar\\baz.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_squash_separator)
{
    test_ptr_success(path = path_new(LIT("\\foo\\\\bar\\\\\\baz.txt"), PATH_STYLE_WINDOWS));
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\foo\\bar\\baz.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\foo\\bar\\baz.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_separator_mix)
{
    test_ptr_success(path = path_new(LIT("/foo\\bar/baz.txt"), PATH_STYLE_WINDOWS));
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\foo\\bar\\baz.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\foo\\bar\\baz.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_invalid_letter)
{
    test_ptr_error(path_new(LIT("1:\\foo.txt"), PATH_STYLE_WINDOWS), E_PATH_MALFORMED);
}

TEST_CASE(path_new_windows_drive_relative)
{
    test_ptr_success(path = path_new(LIT("z:"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_absolute)
{
    test_ptr_success(path = path_new(LIT("z:\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_current_relative)
{
    test_ptr_success(path = path_new(LIT("z:."), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_current_absolute)
{
    test_ptr_success(path = path_new(LIT("z:\\."), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_parent_relative)
{
    test_ptr_success(path = path_new(LIT("z:.."), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:.."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:..");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_parent_absolute)
{
    test_ptr_success(path = path_new(LIT("z:\\.."), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:\\.."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:\\..");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_file_relative)
{
    test_ptr_success(path = path_new(LIT("z:foo.txt"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:foo.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:foo.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_file_absolute)
{
    test_ptr_success(path = path_new(LIT("z:\\foo.txt"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:\\foo.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:\\foo.txt");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_path_relative)
{
    test_ptr_success(path = path_new(LIT("z:.\\foo\\bar\\..\\.baz.boz"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:foo\\bar\\..\\.baz.boz"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:foo\\bar\\..\\.baz.boz");
    path_free(path);
    str_unref(str);
}

TEST_SETUP(path_new_drive_path_absolute)
{
    test_ptr_success(path = path_new(LIT("z:\\foo\\bar\\..\\.baz.boz"), PATH_STYLE_WINDOWS));
}

TEST_CASE_FIXTURE(path_new_windows_drive_path_absolute, path_new_drive_path_absolute, path_free)
{
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:\\foo\\bar\\..\\.baz.boz"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:\\foo\\bar\\..\\.baz.boz");
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_dir_relative)
{
    test_ptr_success(path = path_new(LIT("z:foo\\bar\\..\\.baz.boz\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:foo\\bar\\..\\.baz.boz\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:foo\\bar\\..\\.baz.boz\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_dir_absolute)
{
    test_ptr_success(path = path_new(LIT("z:\\foo\\bar\\..\\.baz.boz\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:\\foo\\bar\\..\\.baz.boz\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:\\foo\\bar\\..\\.baz.boz\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_dir_current_relative)
{
    test_ptr_success(path = path_new(LIT("z:foo\\bar\\..\\.baz.boz\\."), PATH_STYLE_WINDOWS));
    test_true(path_is_relative(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 5);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:foo\\bar\\..\\.baz.boz\\."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:foo\\bar\\..\\.baz.boz\\.");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_drive_dir_current_absolute)
{
    test_ptr_success(path = path_new(LIT("z:\\foo\\bar\\..\\.baz.boz\\."), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 5);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("z:\\foo\\bar\\..\\.baz.boz\\."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "z:\\foo\\bar\\..\\.baz.boz\\.");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_unc_missing_share)
{
    test_ptr_error(path_new(LIT("\\\\host"), PATH_STYLE_WINDOWS), E_PATH_MALFORMED);
}

TEST_CASE(path_new_windows_unc)
{
    test_ptr_success(path = path_new(LIT("\\\\host\\share"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_UNC);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\host\\share"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\share");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_unc_root)
{
    test_ptr_success(path = path_new(LIT("\\\\host\\share\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_UNC);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\host\\share"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\share");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_unc_current)
{
    test_ptr_success(path = path_new(LIT("\\\\host\\share\\."), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_UNC);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\host\\share"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\share");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_unc_parent)
{
    test_ptr_success(path = path_new(LIT("\\\\host\\share\\.."), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_UNC);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\host\\share\\.."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\share\\..");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_unc_file)
{
    test_ptr_success(path = path_new(LIT("\\\\host\\share\\foo.txt"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_UNC);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\host\\share\\foo.txt"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\share\\foo.txt");
    path_free(path);
    str_unref(str);
}

TEST_SETUP(path_new_unc_path)
{
    test_ptr_success(path = path_new(LIT("\\\\host\\share\\foo\\bar\\..\\.baz.boz"), PATH_STYLE_WINDOWS));
}

TEST_CASE_FIXTURE(path_new_windows_unc_path, path_new_unc_path, path_free)
{
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_UNC);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\host\\share\\foo\\bar\\..\\.baz.boz"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\share\\foo\\bar\\..\\.baz.boz");
    str_unref(str);
}

TEST_CASE(path_new_windows_unc_dir)
{
    test_ptr_success(path = path_new(LIT("\\\\host\\share\\foo\\bar\\..\\.baz.boz\\"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_UNC);
    test_uint_eq(path_depth(path), 4);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\host\\share\\foo\\bar\\..\\.baz.boz\\"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\share\\foo\\bar\\..\\.baz.boz\\");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_unc_dir_current)
{
    test_ptr_success(path = path_new(LIT("\\\\host\\share\\foo\\bar\\..\\.baz.boz\\."), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_UNC);
    test_uint_eq(path_depth(path), 5);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\host\\share\\foo\\bar\\..\\.baz.boz\\."));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\share\\foo\\bar\\..\\.baz.boz\\.");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_win32_file_invalid)
{
    test_ptr_error(path_new(LIT("\\\\?\\foo"), PATH_STYLE_WINDOWS), E_PATH_MALFORMED);
}

TEST_CASE(path_new_windows_win32_file_drive)
{
    test_ptr_success(path = path_new(LIT("\\\\?\\p:"), PATH_STYLE_WINDOWS));
    test_false(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DRIVE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("p:"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "p:");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_win32_file_unc)
{
    test_ptr_success(path = path_new(LIT("\\\\?\\UNC\\host\\share"), PATH_STYLE_WINDOWS));
    test_true(path_is_absolute(path));
    test_true(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_UNC);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\host\\share"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\share");
    path_free(path);
    str_unref(str);
}

TEST_CASE(path_new_windows_win32_device_missing)
{
    test_ptr_error(path_new(LIT("\\\\.\\"), PATH_STYLE_WINDOWS), E_PATH_MALFORMED);
}

TEST_CASE(path_new_windows_win32_device_missing_name)
{
    test_ptr_error(path_new(LIT("\\\\.\\1"), PATH_STYLE_WINDOWS), E_PATH_MALFORMED);
}

TEST_CASE(path_new_windows_win32_device_missing_ident)
{
    test_ptr_error(path_new(LIT("\\\\.\\x1x"), PATH_STYLE_WINDOWS), E_PATH_MALFORMED);
}

TEST_SETUP(path_new_device)
{
    test_ptr_success(path = path_new(LIT("\\\\.\\com42"), PATH_STYLE_WINDOWS));
}

TEST_CASE_FIXTURE(path_new_windows_win32_device, path_new_device, path_free)
{
    test_true(path_is_absolute(path));
    test_false(path_is_directory(path));
    test_uint_eq(path_type(path), PATH_TYPE_DEVICE);
    test_uint_eq(path_depth(path), 1);
    test_uint_eq(path_len(path, PATH_STYLE_WINDOWS), strlen("\\\\.\\com42"));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\.\\com42");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_reset_invalid_magic, SIGABRT)
{
    path_reset((path_ct)&not_a_path);
}

TEST_CASE_FIXTURE(path_reset, path_new_path_absolute, path_free)
{
    test_void(path_reset(path));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), ".");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_is_absolute_invalid_magic, SIGABRT)
{
    path_is_absolute((path_ct)&not_a_path);
}

TEST_CASE_SIGNAL(path_is_relative_invalid_magic, SIGABRT)
{
    path_is_relative((path_ct)&not_a_path);
}

TEST_CASE_SIGNAL(path_type_invalid_magic, SIGABRT)
{
    path_type((path_ct)&not_a_path);
}

TEST_CASE_SIGNAL(path_depth_invalid_magic, SIGABRT)
{
    path_depth((path_ct)&not_a_path);
}

TEST_CASE_SIGNAL(path_len_invalid_magic, SIGABRT)
{
    path_len((path_ct)&not_a_path, PATH_STYLE_SYSTEM);
}

TEST_CASE_SIGNAL(path_set_invalid_magic, SIGABRT)
{
    path_set((path_ct)&not_a_path, LIT("foo.txt"), PATH_STYLE_SYSTEM);
}

TEST_CASE_FIXTURE_SIGNAL(path_set_invalid_style, path_new_path_absolute, path_free, SIGABRT)
{
    path_set(path, LIT("foo.txt"), 999);
}

TEST_CASE_SIGNAL(path_set_drive_invalid_magic, SIGABRT)
{
    path_set_drive((path_ct)&not_a_path, 'x');
}

TEST_CASE_FIXTURE(path_set_drive_invalid_type, path_new_path_absolute, path_free)
{
    test_ptr_error(path_set_drive(path, 'x'), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE(path_set_drive_invalid_letter, path_new_drive_path_absolute, path_free)
{
    test_ptr_error(path_set_drive(path, '1'), E_PATH_INVALID_DRIVE_LETTER);
}

TEST_CASE_FIXTURE(path_set_drive, path_new_drive_path_absolute, path_free)
{
    test_ptr_success(path_set_drive(path, 'x'));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "x:\\foo\\bar\\..\\.baz.boz");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_set_unc_invalid_magic, SIGABRT)
{
    path_set_unc((path_ct)&not_a_path, LIT("mail"), LIT("spam"));
}

TEST_CASE_FIXTURE(path_set_unc_invalid_type, path_new_path_absolute, path_free)
{
    test_ptr_error(path_set_unc(path, LIT("mail"), LIT("spam")), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE_SIGNAL(path_set_unc_invalid_host1, path_new_unc_path, path_free, SIGABRT)
{
    path_set_unc(path, NULL, LIT("spam"));
}

TEST_CASE_FIXTURE(path_set_unc_invalid_host2, path_new_unc_path, path_free)
{
    test_ptr_error(path_set_unc(path, LIT(""), LIT("spam")), E_PATH_INVALID_UNC_HOST);
}

TEST_CASE_FIXTURE_SIGNAL(path_set_unc_invalid_share1, path_new_unc_path, path_free, SIGABRT)
{
    path_set_unc(path, LIT("mail"), NULL);
}

TEST_CASE_FIXTURE(path_set_unc_invalid_share2, path_new_unc_path, path_free)
{
    test_ptr_error(path_set_unc(path, LIT("mail"), LIT("")), E_PATH_INVALID_UNC_SHARE);
}

TEST_CASE_FIXTURE(path_set_unc, path_new_unc_path, path_free)
{
    test_ptr_success(path_set_unc(path, LIT("mail"), LIT("spam")));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\mail\\spam\\foo\\bar\\..\\.baz.boz");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_set_unc_share_invalid_magic, SIGABRT)
{
    path_set_unc_share((path_ct)&not_a_path, LIT("stuff"));
}

TEST_CASE_FIXTURE(path_set_unc_share_invalid_type, path_new_path_absolute, path_free)
{
    test_ptr_error(path_set_unc_share(path, LIT("stuff")), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE_SIGNAL(path_set_unc_share_invalid_share1, path_new_unc_path, path_free, SIGABRT)
{
    path_set_unc_share(path, NULL);
}

TEST_CASE_FIXTURE(path_set_unc_share_invalid_share2, path_new_unc_path, path_free)
{
    test_ptr_error(path_set_unc_share(path, LIT("")), E_PATH_INVALID_UNC_SHARE);
}

TEST_CASE_FIXTURE(path_set_unc_share, path_new_unc_path, path_free)
{
    test_ptr_success(path_set_unc_share(path, LIT("stuff")));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\host\\stuff\\foo\\bar\\..\\.baz.boz");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_set_device_invalid_magic, SIGABRT)
{
    path_set_device((path_ct)&not_a_path, LIT("flux"), 88);
}

TEST_CASE_FIXTURE(path_set_device_invalid_type, path_new_path_absolute, path_free)
{
    test_ptr_error(path_set_device(path, LIT("flux"), 88), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE_SIGNAL(path_set_device_invalid_name1, path_new_device, path_free, SIGABRT)
{
    path_set_device(path, NULL, 88);
}

TEST_CASE_FIXTURE(path_set_device_invalid_name2, path_new_device, path_free)
{
    test_ptr_error(path_set_device(path, LIT(""), 88), E_PATH_INVALID_DEVICE_NAME);
}

TEST_CASE_FIXTURE(path_set_device, path_new_device, path_free)
{
    test_ptr_success(path_set_device(path, LIT("flux"), 88));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\.\\flux88");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_set_device_ident_invalid_magic, SIGABRT)
{
    path_set_device_ident((path_ct)&not_a_path, 123);
}

TEST_CASE_FIXTURE(path_set_device_ident_invalid_type, path_new_path_absolute, path_free)
{
    test_ptr_error(path_set_device_ident(path, 123), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE(path_set_device_ident, path_new_device, path_free)
{
    test_ptr_success(path_set_device_ident(path, 123));
    test_ptr_success(str = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(str), "\\\\.\\com123");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_set_suffix_invalid_magic, SIGABRT)
{
    path_set_suffix((path_ct)&not_a_path, LIT("pdf"));
}

TEST_CASE_FIXTURE_SIGNAL(path_set_suffix_invalid_suffix1, path_new_file_absolute, path_free, SIGABRT)
{
    path_set_suffix(path, NULL);
}

TEST_CASE_FIXTURE(path_set_suffix_invalid_suffix2, path_new_file_absolute, path_free)
{
    test_ptr_error(path_set_suffix(path, LIT("")), E_PATH_INVALID_SUFFIX);
}

TEST_CASE_FIXTURE(path_set_suffix_root, path_new_root, path_free)
{
    test_ptr_error(path_set_suffix(path, LIT("pdf")), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_set_suffix_current, path_new_current, path_free)
{
    test_ptr_error(path_set_suffix(path, LIT("pdf")), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_set_suffix_parent, path_new_parent, path_free)
{
    test_ptr_error(path_set_suffix(path, LIT("pdf")), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_set_suffix_path, path_new_path_absolute, path_free)
{
    test_ptr_success(path_set_suffix(path, LIT("pdf")));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.pdf");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_set_suffix_dir, path_new_dir_absolute, path_free)
{
    test_ptr_error(path_set_suffix(path, LIT("pdf")), E_PATH_INVALID_PATH);
}

TEST_CASE_SIGNAL(path_add_suffix_invalid_magic, SIGABRT)
{
    path_add_suffix((path_ct)&not_a_path, LIT("pdf"));
}

TEST_CASE_FIXTURE_SIGNAL(path_add_suffix_invalid_suffix1, path_new_file_absolute, path_free, SIGABRT)
{
    path_add_suffix(path, NULL);
}

TEST_CASE_FIXTURE(path_add_suffix_invalid_suffix2, path_new_file_absolute, path_free)
{
    test_ptr_error(path_add_suffix(path, LIT("")), E_PATH_INVALID_SUFFIX);
}

TEST_CASE_FIXTURE(path_add_suffix_root, path_new_root, path_free)
{
    test_ptr_error(path_add_suffix(path, LIT("pdf")), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_add_suffix_current, path_new_current, path_free)
{
    test_ptr_error(path_add_suffix(path, LIT("pdf")), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_add_suffix_parent, path_new_parent, path_free)
{
    test_ptr_error(path_add_suffix(path, LIT("pdf")), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_add_suffix_path, path_new_path_absolute, path_free)
{
    test_ptr_success(path_add_suffix(path, LIT("pdf")));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz.pdf");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_add_suffix_dir, path_new_dir_absolute, path_free)
{
    test_ptr_error(path_add_suffix(path, LIT("pdf")), E_PATH_INVALID_PATH);
}

TEST_CASE_SIGNAL(path_append_invalid_magic, SIGABRT)
{
    path_append((path_ct)&not_a_path, LIT("blubb"), PATH_STYLE_SYSTEM);
}

TEST_CASE_FIXTURE_SIGNAL(path_append_invalid_str, path_new_path_absolute, path_free, SIGABRT)
{
    path_append(path, NULL, PATH_STYLE_SYSTEM);
}

TEST_CASE_FIXTURE_SIGNAL(path_append_invalid_style, path_new_path_absolute, path_free, SIGABRT)
{
    path_append(path, LIT("blubb"), 999);
}

TEST_CASE_FIXTURE(path_append_empty, path_new_path_absolute, path_free)
{
    test_ptr_error(path_append(path, LIT(""), PATH_STYLE_SYSTEM), E_PATH_MALFORMED);
}

TEST_CASE_FIXTURE(path_append_file_relative, path_new_path_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("blubb"), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 5);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_file_absolute, path_new_path_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("/blubb"), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 5);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_path_relative, path_new_path_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("blubb/./flubb"), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 6);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb/flubb");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_path_absolute, path_new_path_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("/blubb/./flubb"), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 6);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb/flubb");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_current_relative, path_new_path_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("."), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 5);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/.");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_current_absolute, path_new_path_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("/."), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 5);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/.");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_file_relative_to_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("blubb"), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 5);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_file_absolute_to_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("/blubb"), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 5);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_path_relative_to_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("blubb/./flubb"), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 6);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb/flubb");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_path_absolute_to_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("/blubb/./flubb"), PATH_STYLE_POSIX));
    test_uint_eq(path_depth(path), 6);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb/flubb");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_dir_relative_to_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("blubb/./flubb/"), PATH_STYLE_POSIX));
    test_true(path_is_directory(path));
    test_uint_eq(path_depth(path), 6);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb/flubb/");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_dir_absolute_to_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("/blubb/./flubb/"), PATH_STYLE_POSIX));
    test_true(path_is_directory(path));
    test_uint_eq(path_depth(path), 6);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/blubb/flubb/");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_current_relative_to_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("."), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 5);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/.");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_current_absolute_to_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("/."), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 5);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/.");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_append_device, path_new_device, path_free)
{
    test_ptr_error(path_append(path, LIT("blubb"), PATH_STYLE_WINDOWS), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE(path_append_squash_current, path_new_dir_current_absolute, path_free)
{
    test_ptr_success(path_append(path, LIT("foo.txt"), PATH_STYLE_POSIX));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 5);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz.boz/foo.txt");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_drop_invalid_magic, SIGABRT)
{
    path_drop((path_ct)&not_a_path, 1);
}

TEST_CASE_FIXTURE(path_drop, path_new_path_absolute, path_free)
{
    test_ptr_success(path_drop(path, 3));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 1);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_drop_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(path_drop(path, 3));
    test_false(path_is_directory(path));
    test_uint_eq(path_depth(path), 1);
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_drop_suffix_invalid_magic, SIGABRT)
{
    path_drop_suffix((path_ct)&not_a_path);
}

TEST_CASE_FIXTURE(path_drop_suffix_root, path_new_root, path_free)
{
    test_ptr_error(path_drop_suffix(path), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_drop_suffix_current, path_new_current, path_free)
{
    test_ptr_error(path_drop_suffix(path), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_drop_suffix_parent, path_new_parent, path_free)
{
    test_ptr_error(path_drop_suffix(path), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_drop_suffix_path, path_new_path_absolute, path_free)
{
    test_ptr_success(path_drop_suffix(path));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_drop_suffix_dir, path_new_dir_absolute, path_free)
{
    test_ptr_error(path_drop_suffix(path), E_PATH_INVALID_PATH);
}

TEST_CASE_FIXTURE(path_drop_suffix_hidden, path_new_path_absolute, path_free)
{
    test_ptr_success(path_drop_suffix(path));
    test_ptr_success(path_drop_suffix(path));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/../.baz");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_get_invalid_magic, SIGABRT)
{
    path_get((path_ct)&not_a_path, PATH_STYLE_SYSTEM);
}

TEST_CASE_FIXTURE_SIGNAL(path_get_invalid_style, path_new_path_absolute, path_free, SIGABRT)
{
    path_get(path, 999);
}

TEST_CASE_FIXTURE(path_get_drive_posix, path_new_drive_path_absolute, path_free)
{
    test_ptr_error(path_get(path, PATH_STYLE_POSIX), E_PATH_UNSUPPORTED);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), 0);
}

TEST_CASE_FIXTURE(path_get_unc_posix, path_new_unc_path, path_free)
{
    test_ptr_error(path_get(path, PATH_STYLE_POSIX), E_PATH_UNSUPPORTED);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), 0);
}

TEST_CASE_FIXTURE(path_get_device_posix, path_new_device, path_free)
{
    test_ptr_error(path_get(path, PATH_STYLE_POSIX), E_PATH_UNSUPPORTED);
    test_uint_eq(path_len(path, PATH_STYLE_POSIX), 0);
}

TEST_CASE_SIGNAL(path_get_drive_letter_invalid_magic, SIGABRT)
{
    path_get_drive_letter((path_ct)&not_a_path);
}

TEST_CASE_FIXTURE(path_get_drive_letter_invalid_type, path_new_path_absolute, path_free)
{
    test_rc_error(path_get_drive_letter(path), '\0', E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE(path_get_drive_letter, path_new_drive_path_absolute, path_free)
{
    test_rc_success(path_get_drive_letter(path), 'z');
}

TEST_CASE_SIGNAL(path_get_unc_host_invalid_magic, SIGABRT)
{
    path_get_unc_host((path_ct)&not_a_path);
}

TEST_CASE_FIXTURE(path_get_unc_host_invalid_type, path_new_path_absolute, path_free)
{
    test_ptr_error(path_get_unc_host(path), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE(path_get_unc_host, path_new_unc_path, path_free)
{
    test_ptr_success(cstr = path_get_unc_host(path));
    test_str_eq(str_c(cstr), "host");
}

TEST_CASE_SIGNAL(path_get_unc_share_invalid_magic, SIGABRT)
{
    path_get_unc_share((path_ct)&not_a_path);
}

TEST_CASE_FIXTURE(path_get_unc_share_invalid_type, path_new_path_absolute, path_free)
{
    test_ptr_error(path_get_unc_share(path), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE(path_get_unc_share, path_new_unc_path, path_free)
{
    test_ptr_success(cstr = path_get_unc_share(path));
    test_str_eq(str_c(cstr), "share");
}

TEST_CASE_SIGNAL(path_get_device_name_invalid_magic, SIGABRT)
{
    path_get_device_name((path_ct)&not_a_path);
}

TEST_CASE_FIXTURE(path_get_device_name_invalid_type, path_new_path_absolute, path_free)
{
    test_ptr_error(path_get_device_name(path), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE(path_get_device_name, path_new_device, path_free)
{
    test_ptr_success(cstr = path_get_device_name(path));
    test_str_eq(str_c(cstr), "com");
}

TEST_CASE_SIGNAL(path_get_device_ident_invalid_magic, SIGABRT)
{
    path_get_device_ident((path_ct)&not_a_path);
}

TEST_CASE_FIXTURE(path_get_device_ident_invalid_type, path_new_path_absolute, path_free)
{
    test_int_error(path_get_device_ident(path), E_PATH_INVALID_TYPE);
}

TEST_CASE_FIXTURE(path_get_device_ident, path_new_device, path_free)
{
    test_rc_success(path_get_device_ident(path), 42);
}

TEST_CASE_SIGNAL(path_get_suffix_invalid_magic, SIGABRT)
{
    path_get_suffix((path_ct)&not_a_path);
}

TEST_CASE_FIXTURE(path_get_suffix_root, path_new_root, path_free)
{
    test_ptr_success(str = path_get_suffix(path));
    test_str_eq(str_c(str), "");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_get_suffix_current, path_new_current, path_free)
{
    test_ptr_success(str = path_get_suffix(path));
    test_str_eq(str_c(str), "");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_get_suffix_parent, path_new_parent, path_free)
{
    test_ptr_success(str = path_get_suffix(path));
    test_str_eq(str_c(str), "");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_get_suffix_file, path_new_file_absolute, path_free)
{
    test_ptr_success(str = path_get_suffix(path));
    test_str_eq(str_c(str), "txt");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_get_suffix_path, path_new_path_absolute, path_free)
{
    test_ptr_success(str = path_get_suffix(path));
    test_str_eq(str_c(str), "boz");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_get_suffix_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(str = path_get_suffix(path));
    test_str_eq(str_c(str), "");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_basename_invalid_magic, SIGABRT)
{
    path_basename((path_ct)&not_a_path, PATH_STYLE_SYSTEM);
}

TEST_CASE_FIXTURE_SIGNAL(path_basename_invalid_style, path_new_path_absolute, path_free, SIGABRT)
{
    path_basename(path, 999);
}

TEST_CASE_FIXTURE(path_basename_file_relative, path_new_file_relative, path_free)
{
    test_ptr_success(str = path_basename(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "foo.txt");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_basename_file_absolute, path_new_file_absolute, path_free)
{
    test_ptr_success(str = path_basename(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "foo.txt");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_basename_path, path_new_path_absolute, path_free)
{
    test_ptr_success(str = path_basename(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), ".baz.boz");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_basename_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(str = path_basename(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), ".baz.boz");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_basename_root, path_new_root, path_free)
{
    test_ptr_success(str = path_basename(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_basename_current, path_new_current, path_free)
{
    test_ptr_success(str = path_basename(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), ".");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_basename_parent, path_new_parent, path_free)
{
    test_ptr_success(str = path_basename(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "..");
    str_unref(str);
}

TEST_CASE_SIGNAL(path_dirname_invalid_magic, SIGABRT)
{
    path_dirname((path_ct)&not_a_path, PATH_STYLE_SYSTEM);
}

TEST_CASE_FIXTURE_SIGNAL(path_dirname_invalid_style, path_new_path_absolute, path_free, SIGABRT)
{
    path_dirname(path, 999);
}

TEST_CASE_FIXTURE(path_dirname_file_relative, path_new_file_relative, path_free)
{
    test_ptr_success(str = path_dirname(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), ".");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_dirname_file_absolute, path_new_file_absolute, path_free)
{
    test_ptr_success(str = path_dirname(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_dirname_path, path_new_path_absolute, path_free)
{
    test_ptr_success(str = path_dirname(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/..");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_dirname_dir, path_new_dir_absolute, path_free)
{
    test_ptr_success(str = path_dirname(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/foo/bar/..");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_dirname_root, path_new_root, path_free)
{
    test_ptr_success(str = path_dirname(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_dirname_current, path_new_current, path_free)
{
    test_ptr_success(str = path_dirname(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), ".");
    str_unref(str);
}

TEST_CASE_FIXTURE(path_dirname_parent, path_new_parent, path_free)
{
    test_ptr_success(str = path_dirname(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "..");
    str_unref(str);
}

test_suite_ct test_suite_path(void)
{
    return test_suite_new_with_cases("path"
        , test_case_new(path_new_invalid_str)
        , test_case_new(path_new_invalid_style)
        , test_case_new(path_new_empty)
        
        , test_case_new(path_new_posix_root)
        , test_case_new(path_new_posix_root2)
        , test_case_new(path_new_posix_root3)
        , test_case_new(path_new_posix_current)
        , test_case_new(path_new_posix_current_dir)
        , test_case_new(path_new_posix_current_absolute)
        , test_case_new(path_new_posix_current_absolute_dir)
        , test_case_new(path_new_posix_parent)
        , test_case_new(path_new_posix_parent_dir)
        , test_case_new(path_new_posix_parent_absolute)
        , test_case_new(path_new_posix_parent_absolute_dir)
        , test_case_new(path_new_posix_parent_current)
        , test_case_new(path_new_posix_parent_current_dir)
        , test_case_new(path_new_posix_file_relative)
        , test_case_new(path_new_posix_file_absolute)
        , test_case_new(path_new_posix_path_relative)
        , test_case_new(path_new_posix_path_absolute)
        , test_case_new(path_new_posix_current_file)
        , test_case_new(path_new_posix_current_path)
        , test_case_new(path_new_posix_dir_relative)
        , test_case_new(path_new_posix_dir_absolute)
        , test_case_new(path_new_posix_dir_current_relative)
        , test_case_new(path_new_posix_dir_current_absolute)
        
        , test_case_new(path_new_posix_squash_current)
        , test_case_new(path_new_posix_squash_separator)
        
        , test_case_new(path_new_windows_root)
        , test_case_new(path_new_windows_root2)
        , test_case_new(path_new_windows_root3)
        , test_case_new(path_new_windows_current)
        , test_case_new(path_new_windows_current_dir)
        , test_case_new(path_new_windows_current_absolute)
        , test_case_new(path_new_windows_current_absolute_dir)
        , test_case_new(path_new_windows_parent)
        , test_case_new(path_new_windows_parent_dir)
        , test_case_new(path_new_windows_parent_absolute)
        , test_case_new(path_new_windows_parent_absolute_dir)
        , test_case_new(path_new_windows_parent_current)
        , test_case_new(path_new_windows_parent_current_dir)
        , test_case_new(path_new_windows_file_relative)
        , test_case_new(path_new_windows_file_absolute)
        , test_case_new(path_new_windows_path_relative)
        , test_case_new(path_new_windows_path_absolute)
        , test_case_new(path_new_windows_current_file)
        , test_case_new(path_new_windows_current_path)
        , test_case_new(path_new_windows_dir_relative)
        , test_case_new(path_new_windows_dir_absolute)
        , test_case_new(path_new_windows_dir_current_relative)
        , test_case_new(path_new_windows_dir_current_absolute)
        
        , test_case_new(path_new_windows_squash_current)
        , test_case_new(path_new_windows_squash_separator)
        , test_case_new(path_new_windows_separator_mix)
        
        , test_case_new(path_new_windows_drive_invalid_letter)
        , test_case_new(path_new_windows_drive_relative)
        , test_case_new(path_new_windows_drive_absolute)
        , test_case_new(path_new_windows_drive_current_relative)
        , test_case_new(path_new_windows_drive_current_absolute)
        , test_case_new(path_new_windows_drive_parent_relative)
        , test_case_new(path_new_windows_drive_parent_absolute)
        , test_case_new(path_new_windows_drive_file_relative)
        , test_case_new(path_new_windows_drive_file_absolute)
        , test_case_new(path_new_windows_drive_path_relative)
        , test_case_new(path_new_windows_drive_path_absolute)
        , test_case_new(path_new_windows_drive_dir_relative)
        , test_case_new(path_new_windows_drive_dir_absolute)
        , test_case_new(path_new_windows_drive_dir_current_relative)
        , test_case_new(path_new_windows_drive_dir_current_absolute)
        
        , test_case_new(path_new_windows_unc_missing_share)
        , test_case_new(path_new_windows_unc)
        , test_case_new(path_new_windows_unc_root)
        , test_case_new(path_new_windows_unc_current)
        , test_case_new(path_new_windows_unc_parent)
        , test_case_new(path_new_windows_unc_file)
        , test_case_new(path_new_windows_unc_path)
        , test_case_new(path_new_windows_unc_dir)
        , test_case_new(path_new_windows_unc_dir_current)
        
        , test_case_new(path_new_windows_win32_file_invalid)
        , test_case_new(path_new_windows_win32_file_drive)
        , test_case_new(path_new_windows_win32_file_unc)
        
        , test_case_new(path_new_windows_win32_device_missing)
        , test_case_new(path_new_windows_win32_device_missing_name)
        , test_case_new(path_new_windows_win32_device_missing_ident)
        , test_case_new(path_new_windows_win32_device)
        
        , test_case_new(path_reset_invalid_magic)
        , test_case_new(path_reset)
        
        , test_case_new(path_is_absolute_invalid_magic)
        , test_case_new(path_is_relative_invalid_magic)
        , test_case_new(path_type_invalid_magic)
        , test_case_new(path_depth_invalid_magic)
        , test_case_new(path_len_invalid_magic)
        
        , test_case_new(path_set_invalid_magic)
        , test_case_new(path_set_invalid_style)
        
        , test_case_new(path_set_drive_invalid_magic)
        , test_case_new(path_set_drive_invalid_type)
        , test_case_new(path_set_drive_invalid_letter)
        , test_case_new(path_set_drive)
        
        , test_case_new(path_set_unc_invalid_magic)
        , test_case_new(path_set_unc_invalid_type)
        , test_case_new(path_set_unc_invalid_host1)
        , test_case_new(path_set_unc_invalid_host2)
        , test_case_new(path_set_unc_invalid_share1)
        , test_case_new(path_set_unc_invalid_share2)
        , test_case_new(path_set_unc)
        , test_case_new(path_set_unc_share_invalid_magic)
        , test_case_new(path_set_unc_share_invalid_type)
        , test_case_new(path_set_unc_share_invalid_share1)
        , test_case_new(path_set_unc_share_invalid_share2)
        , test_case_new(path_set_unc_share)
        
        , test_case_new(path_set_device_invalid_magic)
        , test_case_new(path_set_device_invalid_type)
        , test_case_new(path_set_device_invalid_name1)
        , test_case_new(path_set_device_invalid_name2)
        , test_case_new(path_set_device)
        , test_case_new(path_set_device_ident_invalid_magic)
        , test_case_new(path_set_device_ident_invalid_type)
        , test_case_new(path_set_device_ident)
        
        , test_case_new(path_set_suffix_invalid_magic)
        , test_case_new(path_set_suffix_invalid_suffix1)
        , test_case_new(path_set_suffix_invalid_suffix2)
        , test_case_new(path_set_suffix_root)
        , test_case_new(path_set_suffix_current)
        , test_case_new(path_set_suffix_parent)
        , test_case_new(path_set_suffix_path)
        , test_case_new(path_set_suffix_dir)
        , test_case_new(path_add_suffix_invalid_magic)
        , test_case_new(path_add_suffix_invalid_suffix1)
        , test_case_new(path_add_suffix_invalid_suffix2)
        , test_case_new(path_add_suffix_root)
        , test_case_new(path_add_suffix_current)
        , test_case_new(path_add_suffix_parent)
        , test_case_new(path_add_suffix_path)
        , test_case_new(path_add_suffix_dir)
        
        , test_case_new(path_append_invalid_magic)
        , test_case_new(path_append_invalid_str)
        , test_case_new(path_append_invalid_style)
        , test_case_new(path_append_empty)
        , test_case_new(path_append_file_relative)
        , test_case_new(path_append_file_absolute)
        , test_case_new(path_append_path_relative)
        , test_case_new(path_append_path_absolute)
        , test_case_new(path_append_current_relative)
        , test_case_new(path_append_current_absolute)
        , test_case_new(path_append_file_relative_to_dir)
        , test_case_new(path_append_file_absolute_to_dir)
        , test_case_new(path_append_path_relative_to_dir)
        , test_case_new(path_append_path_absolute_to_dir)
        , test_case_new(path_append_dir_relative_to_dir)
        , test_case_new(path_append_dir_absolute_to_dir)
        , test_case_new(path_append_current_relative_to_dir)
        , test_case_new(path_append_current_absolute_to_dir)
        , test_case_new(path_append_device)
        , test_case_new(path_append_squash_current)
        
        , test_case_new(path_drop_invalid_magic)
        , test_case_new(path_drop)
        , test_case_new(path_drop_dir)
        
        , test_case_new(path_drop_suffix_invalid_magic)
        , test_case_new(path_drop_suffix_root)
        , test_case_new(path_drop_suffix_current)
        , test_case_new(path_drop_suffix_parent)
        , test_case_new(path_drop_suffix_path)
        , test_case_new(path_drop_suffix_dir)
        , test_case_new(path_drop_suffix_hidden)
        
        , test_case_new(path_get_invalid_magic)
        , test_case_new(path_get_invalid_style)
        , test_case_new(path_get_drive_posix)
        , test_case_new(path_get_unc_posix)
        , test_case_new(path_get_device_posix)
        
        , test_case_new(path_get_drive_letter_invalid_magic)
        , test_case_new(path_get_drive_letter_invalid_type)
        , test_case_new(path_get_drive_letter)
        
        , test_case_new(path_get_unc_host_invalid_magic)
        , test_case_new(path_get_unc_host_invalid_type)
        , test_case_new(path_get_unc_host)
        , test_case_new(path_get_unc_share_invalid_magic)
        , test_case_new(path_get_unc_share_invalid_type)
        , test_case_new(path_get_unc_share)
        
        , test_case_new(path_get_device_name_invalid_magic)
        , test_case_new(path_get_device_name_invalid_type)
        , test_case_new(path_get_device_name)
        , test_case_new(path_get_device_ident_invalid_magic)
        , test_case_new(path_get_device_ident_invalid_type)
        , test_case_new(path_get_device_ident)
        
        , test_case_new(path_get_suffix_invalid_magic)
        , test_case_new(path_get_suffix_root)
        , test_case_new(path_get_suffix_current)
        , test_case_new(path_get_suffix_parent)
        , test_case_new(path_get_suffix_file)
        , test_case_new(path_get_suffix_path)
        , test_case_new(path_get_suffix_dir)
        
        , test_case_new(path_basename_invalid_magic)
        , test_case_new(path_basename_invalid_style)
        , test_case_new(path_basename_file_relative)
        , test_case_new(path_basename_file_absolute)
        , test_case_new(path_basename_path)
        , test_case_new(path_basename_dir)
        , test_case_new(path_basename_root)
        , test_case_new(path_basename_current)
        , test_case_new(path_basename_parent)
        
        , test_case_new(path_dirname_invalid_magic)
        , test_case_new(path_dirname_invalid_style)
        , test_case_new(path_dirname_file_relative)
        , test_case_new(path_dirname_file_absolute)
        , test_case_new(path_dirname_path)
        , test_case_new(path_dirname_dir)
        , test_case_new(path_dirname_root)
        , test_case_new(path_dirname_current)
        , test_case_new(path_dirname_parent)
    );
}
