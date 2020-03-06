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

/*static const error_info_st error_infos[] =
{
      ERROR_INFO(E_REGPATH_INVALID_BASE, "Invalid registry base.")
};*/

/*
path_ct path_new(str_const_ct str, path_style_id style)
{
    return error_pass_ptr(path_new_cn(str_c(str), str_len(str), style));
}

path_ct path_new_c(const char *str, path_style_id style)
{
    return error_pass_ptr(path_new_cn(str, strlen(str), style));
}

path_ct path_new_cn(const char *str, size_t len, path_style_id style)
{
    path_ct path;
    
    assert(str);
    
    if(!(path = path_new_current()))
        return error_pass(), NULL;
    
    if(!path_set_cn(path, str, len, style))
        return error_pass(), path_free(path), NULL;
    
    return path;
}

path_ct path_new_current(void)
{
    path_ct path;
    
    if(!(path = calloc(1, sizeof(path_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic(path);
    
    return path;
}

path_ct path_new_parent(void)
{
    path_ct path;
    path_comp_st *comp;
    
    if(!(path = path_new_current()))
        return error_pass(), NULL;
    
    if(!(path->comp = vec_new(1, sizeof(path_comp_st))))
        return error_wrap(), path_free(path), NULL;
    
    comp = vec_push(path->comp);
    comp->len = PATH_COMP_PARENT;
    
    return path;
}

static int path_vec_dup_comp(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    path_comp_st *comp = elem, *ncomp;
    vec_ct nvec = ctx;
    
    if(!(ncomp = vec_push(nvec)))
        return error_wrap(), -1;
    
    if(comp->name && !(ncomp->name = strdup(comp->name)))
        return error_wrap_errno(strdup), vec_pop(nvec), -1;
    
    ncomp->len = comp->len;
    
    return 0;
}

path_ct path_dup(path_const_ct path)
{
    path_ct npath;
    
    assert_magic(path);
    
    if(!(npath = calloc(1, sizeof(path_st))))
        return error_wrap_errno(calloc), NULL;
    
    memcpy(npath, path, sizeof(path_st));
    
    if(path->comp && !vec_is_empty(path->comp))
    {
        if(!(npath->comp = vec_new(vec_size(path->comp), sizeof(path_comp_st))))
            return error_wrap(), free(npath), NULL;
        
        if(vec_fold(path->comp, path_vec_dup_comp, npath->comp))
            return error_wrap(), path_free(npath), NULL;
    }
    
    return npath;
}

static void path_vec_free_comp(vec_const_ct vec, void *elem, void *ctx)
{
    path_comp_st *comp = elem;
    
    if(comp->name)
        free(comp->name);
}

void path_reset(path_ct path)
{
    assert_magic(path);
    
    switch(path->type)
    {
    case PATH_TYPE_UNC:
        if(path->info.unc.host)
            str_unref(path->info.unc.host);
        if(path->info.unc.share)
            str_unref(path->info.unc.share);
        break;
    case PATH_TYPE_DEVICE:
        if(path->info.device.name)
            str_unref(path->info.device.name);
        break;
    default:
        break;
    }
    
    path->type = PATH_TYPE_STANDARD;
    path->absolute = false;
    path->trailing = false;
    
    if(path->comp)
        vec_clear_f(path->comp, path_vec_free_comp, NULL);
}

static void path_clear(path_ct path)
{
    path_reset(path);
    
    if(path->comp)
        vec_free(path->comp);
}

void path_free(path_ct path)
{
    path_clear(path);
    free(path);
}

bool path_is_absolute(path_const_ct path)
{
    assert_magic(path);
    
    return path->absolute;
}

bool path_is_relative(path_const_ct path)
{
    assert_magic(path);
    
    return !path->absolute;
}

bool path_is_directory(path_const_ct path)
{
    assert_magic(path);
    
    switch(path->type)
    {
    case PATH_TYPE_DEVICE:
        return false;
    default:
        return path->trailing || (path->absolute && (!path->comp || vec_is_empty(path->comp)));
    }
}

static bool path_compare_comp(path_const_ct path1, path_const_ct path2, path_style_id style)
{
    size_t c, comps;
    path_comp_st *comp1, *comp2;
    int rc;
    
    comps = vec_size(path1->comp);
    return_value_if_fail(comps == vec_size(path2->comp), false);
    
    for(c=0; c < comps; c++)
    {
        comp1 = vec_at(path1->comp, c);
        comp2 = vec_at(path2->comp, c);
        
        if(comp1->len != comp2->len)
            return false;
        
        if(!comp1->name && !comp2->name)
            continue;
        
        if(!comp1->name || !comp2->name)
            return false;
        
        if(style == PATH_STYLE_POSIX)
            rc = strcmp(comp1->name, comp2->name);
        else
            rc = strcasecmp(comp1->name, comp2->name);
        
        if(rc)
            return false;
    }
    
    return true;
}

bool path_is_equal(path_const_ct path1, path_const_ct path2, path_style_id style)
{
    assert_magic(path1);
    assert_magic(path2);
    
    if(path1->type != path2->type
    || path1->absolute != path2->absolute
    || (style == PATH_STYLE_POSIX && path1->trailing != path2->trailing))
        return false;
    
    switch(path1->type)
    {
    case PATH_TYPE_STANDARD:
        return path_compare_comp(path1, path2, style);
    case PATH_TYPE_DRIVE:
        return toupper(path1->info.drive.letter) == toupper(path2->info.drive.letter)
            && path_compare_comp(path1, path2, PATH_STYLE_WINDOWS);
    case PATH_TYPE_UNC:
        return !str_casecmp(path1->info.unc.host, path2->info.unc.host)
            && !str_casecmp(path1->info.unc.share, path2->info.unc.share)
            && path_compare_comp(path1, path2, PATH_STYLE_WINDOWS);
    case PATH_TYPE_DEVICE:
        return !str_casecmp(path1->info.device.name, path2->info.device.name)
            && path1->info.device.id == path2->info.device.id;
    default:
        abort();
    }
}

path_type_id path_type(path_const_ct path)
{
    assert_magic(path);
    
    return path->type;
}

size_t path_depth(path_const_ct path)
{
    assert_magic(path);
    
    return path->comp && !vec_is_empty(path->comp) ? vec_size(path->comp) : 1;
}

size_t path_len(path_const_ct path, path_style_id style)
{
    path_comp_st *comp;
    size_t c, len = 0;
    bool current = false;
    
    assert_magic(path);
    
    if(path->comp && !vec_is_empty(path->comp))
    {
        for(c=0; (comp = vec_at(path->comp, c)); c++)
            len += (c ? 1 : 0) +
                (comp->name ? comp->len :
                comp->len == PATH_COMP_CURRENT ? strlen(path_props[style].current) :
                comp->len == PATH_COMP_PARENT ? strlen(path_props[style].parent) : 0);
    }
    else
        current = true;
    
    if(path->trailing)
        len++; // trailing path separator
    
    switch(path->type)
    {
    case PATH_TYPE_STANDARD:
        return len + (path->absolute ? 1 : current ? strlen(path_props[style].current) : 0);
    case PATH_TYPE_DRIVE:
        return style == PATH_STYLE_WINDOWS ? (path->absolute ? 3 : 2) + len : 0;
    case PATH_TYPE_UNC:
        return style == PATH_STYLE_WINDOWS ?
            (len ? 4 : 3) + str_len(path->info.unc.host) + str_len(path->info.unc.share) + len : 0;
    case PATH_TYPE_DEVICE:
        return style == PATH_STYLE_WINDOWS ?
            4 + str_len(path->info.device.name) + snprintf(NULL, 0, "%zu", path->info.device.id) : 0;
    default:
        abort();
    }
}

const char *path_current(path_style_id style)
{
    assert(style < PATH_STYLES);
    
    return path_props[style].current;
}

const char *path_parent(path_style_id style)
{
    assert(style < PATH_STYLES);
    
    return path_props[style].parent;
}

const char *path_separator(path_style_id style)
{
    assert(style < PATH_STYLES);
    
    return path_props[style].sep;
}

static path_comp_st *path_set_comp(path_comp_st *comp, const char *str, size_t len, path_style_id style)
{
    char *name;
    
    if(len == strlen(path_props[style].current) && !strncmp(str, path_props[style].current, len))
    {
        if(comp->name)
            free(comp->name);
        
        comp->name = NULL;
        comp->len = PATH_COMP_CURRENT;
        
        return comp;
    }
    
    if(len == strlen(path_props[style].parent) && !strncmp(str, path_props[style].parent, len))
    {
        if(comp->name)
            free(comp->name);
        
        comp->name = NULL;
        comp->len = PATH_COMP_PARENT;
        
        return comp;
    }
    
    if(!(name = strndup(str, len)))
        return error_wrap_errno(strndup), NULL;
    
    if(comp->name)
        free(comp->name);
    
    comp->name = name;
    comp->len = len;
    
    return comp;
}

static path_comp_st *path_add_comp(path_ct path, const char *str, size_t len, path_style_id style)
{
    path_comp_st *comp;
    
    // squash 'current' dir if last path component
    if((comp = vec_last(path->comp)) && !comp->name && comp->len == PATH_COMP_CURRENT)
    {
        if(!path_set_comp(comp, str, len, style))
            return error_pass(), NULL;
    }
    else if((comp = vec_push(path->comp)))
    {
        if(!path_set_comp(comp, str, len, style))
            return error_pass(), vec_pop(path->comp), NULL;
    }
    else
        return error_wrap(), NULL;
    
    return comp;
}

static path_ct path_parse_comp(path_ct path, const char *str, size_t len, path_style_id style)
{
    path_comp_st *comp;
    const char *ptr;
    
    if(!len)
        return path;
    
    if(!path->comp && !(path->comp = vec_new(AVG_PATH_COMPS, sizeof(path_comp_st))))
        return error_wrap(), NULL;
    
    for(; (ptr = strnpbrk(str, path_props[style].sep, len)); len -= ptr-str+1, str = ptr+1)
    {
        if(ptr == str)
            continue;
        
        if(!path_add_comp(path, str, ptr - str, style))
            return error_pass(), NULL;
    }
    
    // trailing path separator
    path->trailing = !len && !vec_is_empty(path->comp);
    
    if(len && !path_add_comp(path, str, len, style))
        return error_pass(), NULL;
    
    if((comp = vec_last(path->comp)) && !comp->name && comp->len == PATH_COMP_CURRENT)
    {
        // drop 'current' if only path component or previous component is 'parent'
        if(!(comp = vec_at(path->comp, -2))
        || (!comp->name && comp->len == PATH_COMP_PARENT))
        {
            vec_pop(path->comp);
            
            if(!comp && path->absolute)
                path->trailing = false;
        }
    }
    
    return path;
}

static path_ct path_set_standard(path_ct path, const char *str, size_t len, path_style_id style)
{
    if(len == strlen(path_props[style].current) && !strncasecmp(str, path_props[style].current, len))
        return path;
    
    if(strchr(path_props[style].sep, str[0]))
    {
        path->absolute = true;
        str++;
        len--;
    }
    
    return error_pass_ptr(path_parse_comp(path, str, len, style));
}

static path_ct path_set_posix(path_ct path, const char *str, size_t len)
{
    if(len >= 2
    && strchr(path_props[PATH_STYLE_POSIX].sep, str[0])
    && strchr(path_props[PATH_STYLE_POSIX].sep, str[1])
    && (len == 2 || !strchr(path_props[PATH_STYLE_POSIX].sep, str[2])))
        return error_set(E_PATH_UNSUPPORTED), NULL;
    
    return error_pass_ptr(path_set_standard(path, str, len, PATH_STYLE_POSIX));
}

static path_ct path_set_win_drive(path_ct path, char drive, const char *str, size_t len, path_style_id style)
{
    if(!isalpha(drive))
        return error_set(E_PATH_MALFORMED), NULL;
    
    path->type = PATH_TYPE_DRIVE;
    path->info.drive.letter = drive;
    
    if(len && strchr(path_props[style].sep, str[0]))
        path->absolute = true;
    
    return error_pass_ptr(path_parse_comp(path, str, len, style));
}

static path_ct path_set_win_unc(path_ct path, const char *str, size_t len, path_style_id style)
{
    const char *share, *ptr;
    
    if(!len || !(share = strnpbrk(str, path_props[style].sep, len)) || share == str)
        return error_set(E_PATH_MALFORMED), NULL;
    
    share++;
    len -= share - str;
    
    if(!len || ((ptr = strnpbrk(share, path_props[style].sep, len)) && ptr == share))
        return error_set(E_PATH_MALFORMED), NULL;
    
    path->type = PATH_TYPE_UNC;
    path->absolute = true;
    
    if(!(path->info.unc.host = str_dup_cn(str, share-str-1)))
        return error_wrap(), NULL;
    
    if(!(path->info.unc.share = str_dup_cn(share, ptr ? (size_t)(ptr-share) : len)))
        return error_wrap(), NULL;
    
    if(!ptr)
        return path;
    
    ptr++;
    len -= ptr - share;
    
    return error_pass_ptr(path_parse_comp(path, ptr, len, style));
}

static path_ct path_set_win32_file(path_ct path, const char *str, size_t len)
{
    if(len >= 2 && str[1] == ':')
        return error_pass_ptr(path_set_win_drive(path, str[0], str+2, len-2, PATH_STYLE_WINDOWS));
    
    if(len >= 4 && !strncasecmp(str, "UNC", 3) && strchr(path_props[PATH_STYLE_WINDOWS].sep, str[3]))
        return error_pass_ptr(path_set_win_unc(path, str+4, len-4, PATH_STYLE_WINDOWS));
    
    return error_set(E_PATH_MALFORMED), NULL;
}

static path_ct path_set_win32_device(path_ct path, const char *str, size_t len)
{
    const char *ptr;
    
    if(len < 2 || strnpbrk(str, path_props[PATH_STYLE_WINDOWS].sep, len))
        return error_set(E_PATH_MALFORMED), NULL;
    
    if(!(ptr = memrwhile(str, len, isdigit)) || ptr == str+len-1)
        return error_set(E_PATH_MALFORMED), NULL;
    
    ptr++;
    
    if(strn2uz(&path->info.device.id, ptr, len-(ptr-str), 10) < 0)
        return error_set(E_PATH_MALFORMED), NULL;
    
    path->type = PATH_TYPE_DEVICE;
    path->absolute = true;
    
    if(!(path->info.device.name = str_dup_cn(str, ptr-str)))
        return error_wrap(), NULL;
    
    return path;
}

static path_ct path_set_windows(path_ct path, const char *str, size_t len)
{
    const char *ptr;
    
    if(len == 1)
        return error_pass_ptr(path_set_standard(path, str, len, PATH_STYLE_WINDOWS));
    
    if(str[1] == ':')
        return error_pass_ptr(path_set_win_drive(path, str[0], str+2, len-2, PATH_STYLE_WINDOWS));
    
    if(!(ptr = strchr(path_props[PATH_STYLE_WINDOWS].sep, str[0])) || str[1] != ptr[0])
        return error_pass_ptr(path_set_standard(path, str, len, PATH_STYLE_WINDOWS));
    
    if(len == 2)
        return error_set(E_PATH_MALFORMED), NULL;
    
    if(strchr(path_props[PATH_STYLE_WINDOWS].sep, str[2]))
        return error_pass_ptr(path_set_standard(path, str, len, PATH_STYLE_WINDOWS));
    
    if(len == 3 || !strchr(path_props[PATH_STYLE_WINDOWS].sep, str[3]))
        return error_pass_ptr(path_set_win_unc(path, str+2, len-2, PATH_STYLE_WINDOWS));
    
    switch(str[2])
    {
    case '?':
        return error_pass_ptr(path_set_win32_file(path, str+4, len-4));
    case '.':
        return error_pass_ptr(path_set_win32_device(path, str+4, len-4));
    default:
        return error_pass_ptr(path_set_win_unc(path, str+2, len-2, PATH_STYLE_WINDOWS));
    }
}

path_ct path_set(path_ct path, str_const_ct str, path_style_id style)
{
    return error_pass_ptr(path_set_cn(path, str_c(str), str_len(str), style));
}

path_ct path_set_c(path_ct path, const char *str, path_style_id style)
{
    return error_pass_ptr(path_set_cn(path, str, strlen(str), style));
}

path_ct path_set_cn(path_ct path, const char *str, size_t len, path_style_id style)
{
    path_st tmp = {0}, *npath = &tmp;
    
    assert_magic(path);
    assert(str);
    assert(style < PATH_STYLES);
    
    return_error_if_pass(!len, E_PATH_MALFORMED, NULL);
    
    init_magic(npath);
    
    switch(style)
    {
    case PATH_STYLE_POSIX:   npath = error_pass_ptr(path_set_posix(npath, str, len)); break;
    case PATH_STYLE_WINDOWS: npath = error_pass_ptr(path_set_windows(npath, str, len)); break;
    default:                 abort();
    }
    
    if(!npath)
        return path_clear(&tmp), NULL;
    
    path_clear(path);
    memcpy(path, npath, sizeof(path_st));
    
    return path;
}

path_ct path_set_drive(path_ct path, char letter)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_DRIVE, E_PATH_INVALID_TYPE, NULL);
    return_error_if_fail(isalpha(letter), E_PATH_INVALID_DRIVE_LETTER, NULL);
    
    path->info.drive.letter = letter;
    
    return path;
}

path_ct path_set_unc(path_ct path, str_const_ct host, str_const_ct share)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_UNC, E_PATH_INVALID_TYPE, NULL);
    return_error_if_pass(str_is_empty(host), E_PATH_INVALID_UNC_HOST, NULL);
    return_error_if_pass(str_is_empty(share), E_PATH_INVALID_UNC_SHARE, NULL);
    
    if(!(host = str_ref(host)))
        return error_wrap(), NULL;
    
    if(!(share = str_ref(share)))
        return error_wrap(), str_unref(host), NULL;
    
    if(!str_mark_const(host) || !str_mark_const(share))
        return error_wrap(), str_unref(host), str_unref(share), NULL;
    
    if(path->info.unc.host)
        str_unref(path->info.unc.host);
    if(path->info.unc.share)
        str_unref(path->info.unc.share);
    
    path->info.unc.host = host;
    path->info.unc.share = share;
    
    return path;
}

path_ct path_set_unc_share(path_ct path, str_const_ct share)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_UNC, E_PATH_INVALID_TYPE, NULL);
    return_error_if_pass(str_is_empty(share), E_PATH_INVALID_UNC_SHARE, NULL);
    
    if(!(share = str_ref(share)))
        return error_wrap(), NULL;
    
    if(!str_mark_const(share))
        return error_wrap(), str_unref(share), NULL;
    
    if(path->info.unc.share)
        str_unref(path->info.unc.share);
    
    path->info.unc.share = share;
    
    return path;
}

path_ct path_set_device(path_ct path, str_const_ct name, size_t id)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_DEVICE, E_PATH_INVALID_TYPE, NULL);
    return_error_if_pass(str_is_empty(name), E_PATH_INVALID_DEVICE_NAME, NULL);
    
    if(!(name = str_ref(name)))
        return error_wrap(), NULL;
    
    if(!str_mark_const(name))
        return error_wrap(), str_unref(name), NULL;
    
    if(path->info.device.name)
        str_unref(path->info.device.name);
    
    path->info.device.name = name;
    path->info.device.id = id;
    
    return path;
}

path_ct path_set_device_ident(path_ct path, size_t id)
{
    assert_magic(path);
    return_error_if_fail(path->type == PATH_TYPE_DEVICE, E_PATH_INVALID_TYPE, NULL);
    
    path->info.device.id = id;
    
    return path;
}

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
