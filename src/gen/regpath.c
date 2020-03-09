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

#include <ytil/gen/regpath.h>
#include <ytil/gen/path.h>
#include <ytil/magic.h>
#include <ytil/def.h>


enum def
{
      MAGIC = define_magic('R', 'P', 'A')
};

typedef struct regpath
{
    DEBUG_MAGIC
    
    regpath_base_id base;
    path_ct path;
} regpath_st;

typedef struct regpath_base_info
{
    const char *abbr, *name;
} regpath_base_info_st;

static const regpath_base_info_st regpath_base_infos[] =
{
      [REGPATH_CLASSES_ROOT]        = { "HKCR", "HKEY_CLASSES_ROOT" }
    , [REGPATH_CURRENT_CONFIG]      = { "HKCC", "HKEY_CURRENT_CONFIG" }
    , [REGPATH_CURRENT_USER]        = { "HKCU", "HKEY_CURRENT_USER" }
    , [REGPATH_DYN_DATA]            = { "HKDD", "HKEY_DYN_DATA" }
    , [REGPATH_LOCAL_MACHINE]       = { "HKLM", "HKEY_LOCAL_MACHINE" }
    , [REGPATH_PERFORMANCE_DATA]    = { "HKPD", "HKEY_PERFORMANCE_DATA" }
    , [REGPATH_USERS]               = { "HKU",  "HKEY_USERS" }
};

static const path_prop_st regpath_prop = { .sep = "\\" };

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_REGPATH_INVALID_BASE, "Invalid registry base.")
    , ERROR_INFO(E_REGPATH_INVALID_PATH, "Invalid registry path.")
};


static regpath_ct regpath_new_empty(void)
{
    regpath_ct path;
    
    if(!(path = calloc(1, sizeof(regpath_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic(path);
    
    return path;
}

regpath_ct regpath_new(str_const_ct str)
{
    return error_pass_ptr(regpath_new_cn(str_c(str), str_len(str)));
}

regpath_ct regpath_new_c(const char *str)
{
    return error_pass_ptr(regpath_new_cn(str, strlen(str)));
}

regpath_ct regpath_new_cn(const char *str, size_t len)
{
    regpath_ct path;
    
    if(!(path = regpath_new_empty()))
        return error_pass(), NULL;
    
    if(!regpath_set_cn(path, str, len))
        return error_pass(), regpath_free(path), NULL;
    
    return path;
}

regpath_ct regpath_new_with_base(regpath_base_id base, str_const_ct str)
{
    return error_pass_ptr(regpath_new_with_base_cn(base, str_c(str), str_len(str)));
}

regpath_ct regpath_new_with_base_c(regpath_base_id base, const char *str)
{
    return error_pass_ptr(regpath_new_with_base_cn(base, str, strlen(str)));
}

regpath_ct regpath_new_with_base_cn(regpath_base_id base, const char *str, size_t len)
{
    regpath_ct path;
    
    if(!(path = regpath_new_empty()))
        return error_pass(), NULL;
    
    if(!regpath_set_with_base_cn(path, base, str, len))
        return error_pass(), regpath_free(path), NULL;
    
    return path;
}

regpath_ct regpath_dup(regpath_const_ct path)
{
    regpath_ct npath;
    
    assert_magic(path);
    
    if(!(npath = calloc(1, sizeof(regpath_st))))
        return error_wrap_errno(calloc), NULL;
    
    memcpy(npath, path, sizeof(regpath_st));
    
    if(path->path && !(npath->path = path_dup(path->path)))
        return error_wrap(), regpath_free(npath), NULL;
    
    return npath;
}

void regpath_free(regpath_ct path)
{
    if(path->path)
        path_free(path->path);
    
    free(path);
}

bool regpath_is_equal(regpath_const_ct path1, regpath_const_ct path2)
{
    assert_magic(path1);
    assert_magic(path2);
    
    return path1->base == path2->base
        && ((!path1->path && !path2->path)
            || (path1->path && path2->path
                && path_is_equal(path1->path, path2->path, &regpath_prop)));
}

regpath_base_id regpath_base(regpath_const_ct path)
{
    assert_magic(path);
    
    return path->base;
}

size_t regpath_depth(regpath_const_ct path)
{
    assert_magic(path);
    
    return path->path ? path_depth(path->path) : 0;
}

size_t regpath_len(regpath_const_ct path)
{
    assert_magic(path);
    
    return strlen(regpath_base_infos[path->base].name)
        + (path->path ? path_len(path->path, &regpath_prop) : 0);
}

regpath_ct regpath_set(regpath_ct path, str_const_ct str)
{
    return error_pass_ptr(regpath_set_cn(path, str_c(str), str_len(str)));
}

regpath_ct regpath_set_c(regpath_ct path, const char *str)
{
    return error_pass_ptr(regpath_set_cn(path, str, strlen(str)));
}

regpath_ct regpath_set_cn(regpath_ct path, const char *str, size_t len)
{
    return error_pass_ptr(regpath_set_with_base_cn(path, 0, str, len));
}

regpath_ct regpath_with_base_set(regpath_ct path, regpath_base_id base, str_const_ct str)
{
    return error_pass_ptr(regpath_set_with_base_cn(path, base, str_c(str), str_len(str)));
}

regpath_ct regpath_set_with_base_c(regpath_ct path, regpath_base_id base, const char *str)
{
    return error_pass_ptr(regpath_set_with_base_cn(path, base, str, strlen(str)));
}

regpath_ct regpath_set_with_base_cn(regpath_ct path, regpath_base_id base, const char *str, size_t len)
{
    assert_magic(path);
    return_error_if_fail(base < REGPATH_BASES, E_REGPATH_INVALID_BASE, NULL);
    
    if(!str || !len)
    {
        if(path->path)
        {
            path_free(path->path);
            path->path = NULL;
        }
    }
    else if(!path->path)
    {
        if(!(path->path = path_new_cn(str, len, &regpath_prop)))
            return error_wrap(), NULL;
    }
    else
    {
        if(!path_set_cn(path->path, str, len, &regpath_prop))
            return error_wrap(), NULL;
    }
    
    path->base = base;
    
    return path;
}
/*
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
