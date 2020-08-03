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

#include <ytil/gen/fspath.h>
#include <ytil/gen/path.h>
#include <ytil/ext/string.h>
#include <ytil/ext/stdlib.h>
#include <ytil/magic.h>
#include <ytil/def.h>
#include <stdio.h>


enum def
{
      MAGIC = define_magic('F', 'P', 'A')
};

typedef union fspath_info
{
    struct fspath_info_drive
    {
        char letter;
    } drive;
    struct fspath_info_unc
    {
        str_const_ct host, share;
    } unc;
    struct fspath_info_device
    {
        str_const_ct name;
        size_t id;
    } device;
} fspath_info_un;

typedef struct fspath
{
    DEBUG_MAGIC
    
    path_ct path;
    fspath_type_id type;
    fspath_info_un info;
} fspath_st;

static const path_prop_st fspath_props[] =
{
      [FSPATH_STYLE_POSIX]   = { .sep = "/", .current = ".", .parent = "..", .case_sensitive = true }
    , [FSPATH_STYLE_WINDOWS] = { .sep = "\\/", .current = ".", .parent = "..", .case_sensitive = false }
};

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_FSPATH_INVALID_DEVICE_NAME, "Invalid device name.")
    //, ERROR_INFO(E_FSPATH_INVALID_DRIVE_LETTER, "Invalid drive letter.")
    , ERROR_INFO(E_FSPATH_INVALID_STYLE, "Invalid path style.")
    //, ERROR_INFO(E_FSPATH_INVALID_SUFFIX, "Invalid file suffix.")
    //, ERROR_INFO(E_FSPATH_INVALID_TYPE, "Invalid path type.")
    //, ERROR_INFO(E_FSPATH_INVALID_UNC_HOST, "Invalid UNC host.")
    //, ERROR_INFO(E_FSPATH_INVALID_UNC_SHARE, "Invalid UNC share.")
    , ERROR_INFO(E_FSPATH_MALFORMED, "Path malformed.")
    //, ERROR_INFO(E_FSPATH_UNSUPPORTED, "Unsupported path.")
};


static fspath_ct _fspath_new(void)
{
    fspath_ct path;
    
    if(!(path = calloc(1, sizeof(fspath_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic(path);
    
    return path;
}

static fspath_ct fspath_new_empty(void)
{
    fspath_ct path;
    
    if(!(path = _fspath_new()))
        return error_pass(), NULL;
    
    if(!(path->path = path_new_empty()))
        return error_wrap(), free(path), NULL;
    
    return path;
}

fspath_ct fspath_new(str_const_ct str, fspath_style_id style)
{
    fspath_ct path;
    
    assert(str);
    
    if(!(path = fspath_new_empty()))
        return error_pass(), NULL;
    
    if(!fspath_set(path, str, style))
        return error_pass(), fspath_free(path), NULL;
    
    return path;
}

fspath_ct fspath_new_current(void)
{
    fspath_ct path;
    
    if(!(path = _fspath_new()))
        return error_pass(), NULL;
    
    if(!(path->path = path_new_current()))
        return error_pass(), free(path), NULL;
    
    path->type = FSPATH_TYPE_STANDARD;
    
    return path;
}

fspath_ct fspath_new_parent(void)
{
    fspath_ct path;
    
    if(!(path = _fspath_new()))
        return error_pass(), NULL;
    
    if(!(path->path = path_new_parent()))
        return error_pass(), free(path), NULL;
    
    path->type = FSPATH_TYPE_STANDARD;
    
    return path;
}

fspath_ct fspath_new_root(void)
{
    fspath_ct path;
    
    if(!(path = fspath_new_empty()))
        return error_pass(), NULL;
    
    path->type = FSPATH_TYPE_STANDARD;
    path_set_absolute(path->path, true);
    
    return path;
}

fspath_ct fspath_new_windows_drive(char drive, str_const_ct str)
{
    fspath_ct path;
    
    if(!(path = fspath_new_empty()))
        return error_pass(), NULL;
    
    if(!fspath_set_windows_drive(path, drive, str))
        return error_pass(), fspath_free(path), NULL;
    
    return path;
}

fspath_ct fspath_new_windows_unc(str_const_ct host, str_const_ct share, str_const_ct str)
{
    fspath_ct path;
    
    if(!(path = fspath_new_empty()))
        return error_pass(), NULL;
    
    if(!fspath_set_windows_unc(path, host, share, str))
        return error_pass(), fspath_free(path), NULL;
    
    return path;
}

fspath_ct fspath_new_windows_device(str_const_ct name, size_t id)
{
    fspath_ct path;
    
    if(!(path = fspath_new_empty()))
        return error_pass(), NULL;
    
    if(!fspath_set_windows_device(path, name, id))
        return error_pass(), fspath_free(path), NULL;
    
    return path;
}

fspath_ct fspath_dup(fspath_const_ct path)
{
    fspath_ct npath;
    
    assert_magic(path);
    
    if(!(npath = _fspath_new()))
        return error_pass(), NULL;
    
    if(!(npath->path = path_dup(path->path)))
        return error_wrap(), free(npath), NULL;
    
    switch(path->type)
    {
    case FSPATH_TYPE_UNC:
        if(!(npath->info.unc.host  = str_dup(path->info.unc.host))
        || !(npath->info.unc.share = str_dup(path->info.unc.share)))
            return error_wrap(), fspath_free(npath), NULL;
        break;
    case FSPATH_TYPE_DEVICE:
        if(!(npath->info.device.name = str_dup(path->info.device.name)))
            return error_wrap(), fspath_free(npath), NULL;
        break;
    default:
        break;
    }
    
    return npath;
}

static void fspath_free_info(fspath_ct path)
{
    switch(path->type)
    {
    case FSPATH_TYPE_UNC:
        if(path->info.unc.host)
            str_unref(path->info.unc.host);
        if(path->info.unc.share)
            str_unref(path->info.unc.share);
        break;
    case FSPATH_TYPE_DEVICE:
        if(path->info.device.name)
            str_unref(path->info.device.name);
        break;
    default:
        break;
    }
}

void fspath_free(fspath_ct path)
{
    assert_magic(path);
    
    fspath_free_info(path);
    path_free(path->path);
    free(path);
}

bool fspath_is_absolute(fspath_const_ct path)
{
    assert_magic(path);
    
    switch(path->type)
    {
    case FSPATH_TYPE_STANDARD:
    case FSPATH_TYPE_DRIVE:
        return path_is_absolute(path->path);
    case FSPATH_TYPE_UNC:
    case FSPATH_TYPE_DEVICE:
        return true;
    default:
        abort();
    }
}

bool fspath_is_directory(fspath_const_ct path)
{
    assert_magic(path);
    
    switch(path->type)
    {
    case FSPATH_TYPE_STANDARD:
    case FSPATH_TYPE_DRIVE:
    case FSPATH_TYPE_UNC:
        return path_is_trailing(path->path);
    case FSPATH_TYPE_DEVICE:
        return false;
    default:
        abort();
    }
}

bool fspath_is_equal(fspath_const_ct path1, fspath_const_ct path2, fspath_style_id style)
{
    assert_magic(path1);
    assert_magic(path2);
    assert(style < FSPATH_STYLES);
    
    return_value_if_fail(path1->type == path2->type, false);
    
    switch(path1->type)
    {
    case FSPATH_TYPE_STANDARD:
        return path_is_equal(path1->path, path2->path, &fspath_props[style]);
    case FSPATH_TYPE_DRIVE:
        return style == FSPATH_STYLE_WINDOWS
            && toupper(path1->info.drive.letter) == toupper(path2->info.drive.letter)
            && path_is_equal(path1->path, path2->path, &fspath_props[style]);
    case FSPATH_TYPE_UNC:
        return style == FSPATH_STYLE_WINDOWS
            && !str_casecmp(path1->info.unc.host, path2->info.unc.host)
            && !str_casecmp(path1->info.unc.share, path2->info.unc.share)
            && path_is_equal(path1->path, path2->path, &fspath_props[style]);
    case FSPATH_TYPE_DEVICE:
        return style == FSPATH_STYLE_WINDOWS
            && !str_casecmp(path1->info.device.name, path2->info.device.name);
    default:
        abort();
    }
}

fspath_type_id fspath_type(fspath_const_ct path)
{
    assert_magic(path);
    
    return path->type;
}

size_t fspath_depth(fspath_const_ct path)
{
    assert_magic(path);
    
    return path_depth(path->path);
}

size_t fspath_len(fspath_const_ct path, fspath_style_id style)
{
    assert_magic(path);
    assert(style < FSPATH_STYLES);
    
    switch(path->type)
    {
    case FSPATH_TYPE_STANDARD:
        return path_len(path->path, &fspath_props[style]);
    case FSPATH_TYPE_DRIVE:
        return_value_if_fail(style == FSPATH_STYLE_WINDOWS, 0);
        return 2 + path_len(path->path, &fspath_props[style]);
    case FSPATH_TYPE_UNC:
        return_value_if_fail(style == FSPATH_STYLE_WINDOWS, 0);
        return 2 + str_len(path->info.unc.host)
             + 1 + str_len(path->info.unc.share)
             + path_len_nonempty(path->path, &fspath_props[style]);
    case FSPATH_TYPE_DEVICE:
        return_value_if_fail(style == FSPATH_STYLE_WINDOWS, 0);
        return 4 + str_len(path->info.device.name)
             + snprintf(NULL, 0, "%zu", path->info.device.id);
    default:
        abort();
    }
}

const char *fspath_current(fspath_style_id style)
{
    assert(style < FSPATH_STYLES);
    
    return fspath_props[style].current;
}

const char *fspath_parent(fspath_style_id style)
{
    assert(style < FSPATH_STYLES);
    
    return fspath_props[style].parent;
}

const char *fspath_separators(fspath_style_id style)
{
    assert(style < FSPATH_STYLES);
    
    return fspath_props[style].sep;
}

static inline bool fspath_is_posix_sep(char sep)
{
    return !!strchr(fspath_props[FSPATH_STYLE_POSIX].sep, sep);
}

static inline bool fspath_is_win_sep(char sep)
{
    return !!strchr(fspath_props[FSPATH_STYLE_WINDOWS].sep, sep);
}

static fspath_ct fspath_set_standard(fspath_ct path, const char *str, size_t len, fspath_style_id style)
{
    if(!path_set_cn(path->path, str, len, &fspath_props[style]))
        return error_wrap(), NULL;
    
    path->type = FSPATH_TYPE_STANDARD;
    
    return path;
}

static fspath_ct fspath_set_posix(fspath_ct path, const char *str, size_t len)
{
    if(len >= 2 && fspath_is_posix_sep(str[0]) && str[1] == str[0])
    {
        if(len == 2) // "//"
            return error_set(E_FSPATH_MALFORMED), NULL;
        if(str[2] != str[0]) // "//foo" but not "///foo"
            return error_set(E_PATH_UNSUPPORTED), NULL;
    }
    
    return error_pass_ptr(fspath_set_standard(path, str, len, FSPATH_STYLE_POSIX));
}

static fspath_ct fspath_set_win32_file(fspath_ct path, const char *str, size_t len)
{
    if(len >= 2 && str[1] == ':')
        return error_pass_ptr(fspath_set_windows_drive(path, str[0], STRN(str+2, len-2)));
    
    if(len >= 4 && !strncasecmp(str, "UNC", 3) && fspath_is_win_sep(str[3]))
        return error_pass_ptr(fspath_set_windows_unc(path, STRN(str+4, len-4)));
    
    return error_set(E_FSPATH_MALFORMED), NULL;
}

static fspath_ct fspath_set_win32_device(fspath_ct path, const char *str, size_t len)
{
    const char *ptr;
    size_t id;
    str_ct name;
    
    if(len < 2
    || strnpbrk(str, fspath_props[FSPATH_STYLE_WINDOWS].sep, len) // path has separators
    || !(ptr = memrwhile(str, len, isdigit)) // path has only digits
    || ptr == str+len-1) // path has no digits
        return error_set(E_FSPATH_MALFORMED), NULL;
    
    ptr++;
    
    if(strn2uz(&id, ptr, len-(ptr-str), 10) < 0)
        return error_set(E_FSPATH_MALFORMED), NULL;
    
    if(!(name = str_dup_cn(str, ptr-str)))
        return error_wrap(), NULL;
    
    path->type = FSPATH_TYPE_DEVICE;
    path->info.device.name = name;
    path->info.device.id = id;
    
    return path;
}

static fspath_ct fspath_set_windows(fspath_ct path, const char *str, size_t len)
{
    if(!fspath_is_win_sep(str[0])) // "[^/\].*"
    {
        if(len >= 2 && str[1] == ':') // "[^/\]:.*"
            return error_pass_ptr(fspath_set_win_drive(path, str[0], str+2, len-2));
        else
            return error_pass_ptr(fspath_set_standard(path, str, len, FSPATH_STYLE_WINDOWS));
    }
    
    if(str[1] != str[0]) // "(/[^/].*|\\[^\].*)"
        return error_pass_ptr(fspath_set_standard(path, str, len, FSPATH_STYLE_WINDOWS));
    
    if(len == 2) // "(//|\\\\)"
        return error_set(E_FSPATH_MALFORMED), NULL;
    
    if(fspath_is_win_sep(str[2])) // "(//|\\\\)[/\].*"
        return error_pass_ptr(fspath_set_standard(path, str, len, FSPATH_STYLE_WINDOWS));
    
    if(len == 3 || !fspath_is_win_sep(str[3])) // "(//|\\\\)[^/\]([^/\].*)?"
        return error_pass_ptr(fspath_set_win_unc(path, str+2, len-2));
    
    switch(str[2])
    {
    case '?': // "(//|\\\\)\?[/\].*"
        return error_pass_ptr(fspath_set_win32_file(path, str+4, len-4));
    case '.': // "(//|\\\\)\.[/\].*"
        return error_pass_ptr(fspath_set_win32_device(path, str+4, len-4));
    default:  // "(//|\\\\).[/\].*"
        return error_pass_ptr(fspath_set_win_unc(path, str+2, len-2));
    }
}

fspath_ct fspath_set(fspath_ct path, str_const_ct str, fspath_style_id style)
{
    assert_magic(path);
    return_error_if_fail(str && !str_is_empty(str), E_FSPATH_MALFORMED, NULL);
    return_error_if_fail(style < FSPATH_STYLES, E_FSPATH_INVALID_STYLE, NULL);
    
    switch(style)
    {
    case FSPATH_STYLE_POSIX:   path = fspath_set_posix(path, str_bc(str), str_len(str)); break;
    case FSPATH_STYLE_WINDOWS: path = fspath_set_windows(path, str_bc(str), str_len(str)); break;
    default:                   abort();
    }
    
    return error_pass_ptr(path);
}

fspath_ct fspath_set_windows_drive(fspath_ct path, char drive, str_const_ct str)
{
    assert_magic(path);
    return_error_if_fail(isalpha(letter), E_PATH_INVALID_DRIVE_LETTER, NULL);
    
    if(str && !path_set_cn(path->path, str_bc(str), str_len(str), &fspath_props[FSPATH_STYLE_WINDOWS]))
        return error_wrap(), NULL;
    
    path->type = FSPATH_TYPE_DRIVE;
    path->info.drive.letter = drive;
    
    return path;
}

fspath_ct fspath_set_windows_unc(fspath_ct path, str_const_ct str)
{
    const char *str2, *str3;
    str_ct host, share;
    
    if(!(str2 = strnpbrk(str1, fspath_props[FSPATH_STYLE_WINDOWS].sep, len)))
        return error_set(E_FSPATH_MALFORMED), NULL;
    
    str2++;
    len -= str2 - str1;
    
    if(!len)
        return error_set(E_FSPATH_MALFORMED), NULL;
    
    if((str3 = strnpbrk(str2, fspath_props[FSPATH_STYLE_WINDOWS].sep, len)) && str3 == str2)
        return error_set(E_FSPATH_MALFORMED), NULL;
    
    if(!(host = str_dup_cn(str1, str2-str1-1)))
        return error_wrap(), NULL;
    
    if(!(share = str_dup_cn(str2, str3 ? (size_t)(str3-str2) : len)))
        return error_wrap(), str_unref(host), NULL;
    
    if(str3)
    {
        len -= str3 - str2;
        
        if(!path_set_cn(path->path, str3, len, &fspath_props[FSPATH_STYLE_WINDOWS]))
            return error_wrap(), str_unref(host), str_unref(share), NULL;
    }
    
    path->type = FSPATH_TYPE_UNC;
    path->info.unc.host = host;
    path->info.unc.share = share;
    path_set_absolute(path->path, true);
    
    return path;
}

fspath_ct fspath_set_windows_unc(fspath_ct path, str_const_ct host, str_const_ct share, str_const_ct str)
{
    assert_magic(path);
    return_error_if_pass(host && str_is_empty(host), E_PATH_INVALID_UNC_HOST, NULL);
    return_error_if_pass(share && str_is_empty(share), E_PATH_INVALID_UNC_SHARE, NULL);
    
    if(!(host = str_ref(host)))
        return error_wrap(), NULL;
    
    if(!(share = str_ref(share)))
        return error_wrap(), str_unref(host), NULL;
    
    if(str && !path_set(path->path, str, &fspath_props[FSPATH_STYLE_WINDOWS]))
        return error_wrap(), str_unref(host), str_unref(share), NULL;
    
    fspath_free_info(path);
    
    path->type = FSPATH_TYPE_UNC;
    path->info.unc.host = host;
    path->info.unc.share = share;
    
    return path;
}

fspath_ct fspath_set_windows_device(fspath_ct path, str_const_ct name, size_t id)
{
    assert_magic(path);
    return_error_if_pass(name && str_is_empty(name), E_PATH_INVALID_DEVICE_NAME, NULL);
    
    if(!(name = str_ref(name)))
        return error_wrap(), NULL;
    
    fspath_free_info(path);
    path_reset(path->path);
    
    path->type = FSPATH_TYPE_DEVICE;
    path->info.device.name = name;
    path->info.device.id = id;
    
    return path;
}
/*
path_ct path_set_suffix(path_ct path, str_const_ct suffix)
{
    path_comp_st *comp;
    char *name, *ptr;
    
    assert_magic(path);
    return_error_if_pass(str_is_empty(suffix), E_PATH_INVALID_SUFFIX, NULL);
    
    if(path->trailing || !path->comp || !(comp = vec_last(path->comp)) || !comp->name)
        return error_set(E_PATH_INVALID_PATH), NULL;
    
    if((ptr = strrchr(comp->name, '.')))
        name = strdup_printf("%.*s.%s", (int)(ptr-comp->name), comp->name, str_c(suffix));
    else
        name = strdup_printf("%s.%s", comp->name, str_c(suffix));
    
    if(!name)
        return error_wrap_errno(strdup_printf), NULL;
    
    free(comp->name);
    comp->len = (ptr ? (size_t)(ptr-comp->name) : comp->len) + 1 + str_len(suffix);
    comp->name = name;
    
    return path;
}

path_ct path_add_suffix(path_ct path, str_const_ct suffix)
{
    path_comp_st *comp;
    char *name;
    
    assert_magic(path);
    return_error_if_pass(str_is_empty(suffix), E_PATH_INVALID_SUFFIX, NULL);
    
    if(path->trailing || !path->comp || !(comp = vec_last(path->comp)) || !comp->name)
        return error_set(E_PATH_INVALID_PATH), NULL;
    
    if(!(name = strdup_printf("%s.%s", comp->name, str_c(suffix))))
        return error_wrap_errno(strdup_printf), NULL;
    
    free(comp->name);
    comp->name = name;
    comp->len += 1 + str_len(suffix);
    
    return path;
}

path_ct path_append(path_ct path, str_const_ct str, path_style_id style)
{
    return error_pass_ptr(path_append_cn(path, str_c(str), str_len(str), style));
}

path_ct path_append_c(path_ct path, const char *str, path_style_id style)
{
    return error_pass_ptr(path_append_cn(path, str, strlen(str), style));
}

path_ct path_append_cn(path_ct path, const char *str, size_t len, path_style_id style)
{
    assert_magic(path);
    assert(str);
    assert(style < PATH_STYLES);
    
    return_error_if_pass(path->type == PATH_TYPE_DEVICE, E_PATH_INVALID_TYPE, NULL);
    return_error_if_pass(!len, E_PATH_MALFORMED, NULL);
    
    return error_pass_ptr(path_parse_comp(path, str, len, style));
}

path_ct path_drop(path_ct path, size_t n)
{
    assert_magic(path);
    
    if(path->comp)
        vec_pop_fn(path->comp, n, path_vec_free_comp, NULL);
    
    path->trailing = false;
    
    return path;
}

path_ct path_drop_suffix(path_ct path)
{
    path_comp_st *comp;
    char *ptr;
    
    assert_magic(path);
    
    if(path->trailing || !path->comp || !(comp = vec_last(path->comp)) || !comp->name)
        return error_set(E_PATH_INVALID_PATH), NULL;
    
    if(!(ptr = strrchr(comp->name, '.')) || ptr == comp->name)
        return path;
    
    ptr[0] = '\0';
    comp->len = ptr - comp->name;
    
    return path;
}

static str_ct path_cat_comp(str_ct str, path_const_ct path, path_style_id style, bool dirname)
{
    path_comp_st *comp;
    size_t c, comps;
    
    if(!path->comp || vec_is_empty(path->comp))
        return str ? str : error_wrap_ptr(str_dup_f("%s%.1s",
            path_props[style].current, path->trailing ? path_props[style].sep : ""));
    
    if(dirname && vec_size(path->comp) == 1)
    {
        comp = vec_last(path->comp);
        
        if(comp->name || comp->len != PATH_COMP_PARENT)
            return str ? str : error_wrap_ptr(str_new_s(path_props[style].current));
        else if(str)
            return error_wrap_ptr(str_append_c(str, path_props[style].parent));
        else
            return error_wrap_ptr(str_new_s(path_props[style].parent));
    }
    
    if(!str && !(str = str_new_l("")))
        return error_wrap(), NULL;
    
    comps = vec_size(path->comp) - (dirname ? 1 : 0);
    
    if(dirname && (comp = vec_last(path->comp)) && !comp->name && comp->len == PATH_COMP_CURRENT)
        comps--;
    
    for(c=0; c < comps; c++)
    {
        comp = vec_at(path->comp, c);
        
        if(c && !str_append_set(str, 1, path_props[style].sep[0]))
            return error_wrap(), str_unref(str), NULL;
        
        if(comp->name)
        {
            if(!str_append_cn(str, comp->name, comp->len))
                return error_wrap(), str_unref(str), NULL;
        }
        else switch(comp->len)
        {
        case PATH_COMP_CURRENT:
            if(!str_append_c(str, path_props[style].current))
                return error_wrap(), str_unref(str), NULL;
            break;
        case PATH_COMP_PARENT:
            if(!str_append_c(str, path_props[style].parent))
                return error_wrap(), str_unref(str), NULL;
            break;
        default:
            abort();
        }
    }
    
    if(!dirname && path->trailing && !str_append_set(str, 1, path_props[style].sep[0]))
        return error_wrap(), str_unref(str), NULL;
    
    return str;
}

static str_ct path_get_drive(path_const_ct path, path_style_id style)
{
    switch(style)
    {
    case PATH_STYLE_POSIX:
        return error_set(E_PATH_UNSUPPORTED), NULL;
    case PATH_STYLE_WINDOWS:
        if(path->absolute)
            return error_wrap_ptr(str_dup_f("%c:%c", path->info.drive.letter, path_props[style].sep[0]));
        else
            return error_wrap_ptr(str_dup_f("%c:", path->info.drive.letter));
    default:
        abort();
    }
}

static str_ct path_get_unc(path_const_ct path, path_style_id style)
{
    switch(style)
    {
    case PATH_STYLE_POSIX:
        return error_set(E_PATH_UNSUPPORTED), NULL;
    case PATH_STYLE_WINDOWS:
        if(path->comp && !vec_is_empty(path->comp))
            return error_wrap_ptr(str_dup_f("%c%c%s%c%s%c",
                path_props[style].sep[0], path_props[style].sep[0],
                str_c(path->info.unc.host), path_props[style].sep[0],
                str_c(path->info.unc.share), path_props[style].sep[0]));
        else
            return error_wrap_ptr(str_dup_f("%c%c%s%c%s",
                path_props[style].sep[0], path_props[style].sep[0],
                str_c(path->info.unc.host), path_props[style].sep[0],
                str_c(path->info.unc.share)));
    default:
        abort();
    }
}

static str_ct path_get_device(path_const_ct path, path_style_id style)
{
    switch(style)
    {
    case PATH_STYLE_POSIX:
        return error_set(E_PATH_UNSUPPORTED), NULL;
    case PATH_STYLE_WINDOWS:
        return error_wrap_ptr(str_dup_f("%c%c.%c%s%zu",
            path_props[style].sep[0], path_props[style].sep[0], path_props[style].sep[0],
            str_c(path->info.device.name), path->info.device.id));
    default:
        abort();
    }
}

static str_ct _path_get(path_const_ct path, path_style_id style, bool dirname)
{
    str_ct str = NULL;
    
    switch(path->type)
    {
    case PATH_TYPE_STANDARD:
        if(path->absolute && !(str = str_prepare_set(1, path_props[style].sep[0])))
            return error_wrap(), NULL;
        break;
    case PATH_TYPE_DRIVE:
        if(!(str = path_get_drive(path, style)))
            return error_pass(), NULL;
        break;
    case PATH_TYPE_UNC:
        if(!(str = path_get_unc(path, style)))
            return error_pass(), NULL;
        break;
    case PATH_TYPE_DEVICE:
        if(!(str = path_get_device(path, style)))
            return error_pass(), NULL;
        break;
    default:
        abort();
    }
    
    return error_pass_ptr(path_cat_comp(str, path, style, dirname));
}

str_ct path_get(path_const_ct path, path_style_id style)
{
    assert_magic(path);
    assert(style < PATH_STYLES);
    
    return error_pass_ptr(_path_get(path, style, false));
}

char path_get_drive_letter(path_const_ct path)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_DRIVE, E_PATH_INVALID_TYPE, '\0');
    
    return path->info.drive.letter;
}

str_const_ct path_get_unc_host(path_const_ct path)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_UNC, E_PATH_INVALID_TYPE, NULL);
    
    return path->info.unc.host;
}

str_const_ct path_get_unc_share(path_const_ct path)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_UNC, E_PATH_INVALID_TYPE, NULL);
    
    return path->info.unc.share;
}

str_const_ct path_get_device_name(path_const_ct path)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_DEVICE, E_PATH_INVALID_TYPE, NULL);
    
    return path->info.device.name;
}

ssize_t path_get_device_ident(path_const_ct path)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_DEVICE, E_PATH_INVALID_TYPE, -1);
    
    return path->info.device.id;
}

str_ct path_get_suffix(path_const_ct path)
{
    path_comp_st *comp;
    char *suffix;
    
    assert_magic(path);
    
    if(path->trailing || !path->comp || !(comp = vec_last(path->comp))
    || !comp->name || !(suffix = strrchr(comp->name, '.')))
        return error_wrap_ptr(str_dup_c(""));
    
    suffix++;
    
    return error_wrap_ptr(str_dup_cn(suffix, comp->len - (suffix-comp->name)));
}

str_ct path_dirname(path_const_ct path, path_style_id style)
{
    assert_magic(path);
    assert(style < PATH_STYLES);
    
    return error_pass_ptr(_path_get(path, style, true));
}

str_ct path_basename(path_const_ct path, path_style_id style)
{
    path_comp_st *comp;
    
    assert_magic(path);
    assert(style < PATH_STYLES);
    
    if(!path->comp || vec_is_empty(path->comp))
    {
        if(path->absolute)
            return error_wrap_ptr(str_prepare_set(1, path_props[style].sep[0]));
        else
            return error_wrap_ptr(str_new_s(path_props[style].current));
    }
    
    comp = vec_last(path->comp);
    
    if(comp->name)
        return error_wrap_ptr(str_dup_cn(comp->name, comp->len));
    
    switch(comp->len)
    {
    case PATH_COMP_CURRENT: comp = vec_at(path->comp, -2); break;
    case PATH_COMP_PARENT:  return error_wrap_ptr(str_dup_c(path_props[style].parent));
    default:                abort();
    }
    
    if(comp->name)
        return error_wrap_ptr(str_dup_cn(comp->name, comp->len));
    
    switch(comp->len)
    {
    case PATH_COMP_CURRENT: abort();
    case PATH_COMP_PARENT:  return error_wrap_ptr(str_dup_c(path_props[style].parent));
    default:                abort();
    }
}
*/
