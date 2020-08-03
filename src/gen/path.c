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

#include <ytil/gen/path.h>
#include <ytil/con/vec.h>
#include <ytil/ext/string.h>
#include <ytil/magic.h>
#include <ytil/def.h>


enum def
{
      MAGIC          = define_magic('P', 'T', 'H')
    , AVG_PATH_COMPS = 5
};

typedef enum path_comp_type
{
      PATH_COMP_CURRENT = 1
    , PATH_COMP_PARENT  = 2
} path_comp_type_id;

typedef struct path_comp
{
    char *name;
    size_t len;
} path_comp_st;

typedef struct path
{
    DEBUG_MAGIC
    
    vec_ct comp;
    bool absolute, trailing;
} path_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_PATH_INVALID_PROPERTIES, "Invalid path properties.")
    , ERROR_INFO(E_PATH_UNSUPPORTED, "Unsupported path.")
};


path_ct path_new(str_const_ct str, const path_prop_st *prop)
{
    return error_pass_ptr(path_new_cn(str_bc(str), str_len(str), prop));
}

path_ct path_new_c(const char *str, const path_prop_st *prop)
{
    return error_pass_ptr(path_new_cn(str, strlen(str), prop));
}

path_ct path_new_cn(const char *str, size_t len, const path_prop_st *prop)
{
    path_ct path;
    
    if(!(path = path_new_empty()))
        return error_pass(), NULL;
    
    if(!path_set_cn(path, str, len, prop))
        return error_pass(), path_free(path), NULL;
    
    return path;
}

path_ct path_new_empty(void)
{
    path_ct path;
    
    if(!(path = calloc(1, sizeof(path_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic(path);
    
    return path;
}

static path_ct path_new_special(path_comp_type_id type)
{
    path_ct path;
    path_comp_st *comp;
    
    if(!(path = path_new_empty()))
        return error_pass(), NULL;
    
    if(!(path->comp = vec_new(1, sizeof(path_comp_st))))
        return error_wrap(), path_free(path), NULL;
    
    comp = vec_push(path->comp);
    comp->len = type;
    
    return path;
}

path_ct path_new_current(void)
{
    return error_pass_ptr(path_new_special(PATH_COMP_CURRENT));
}

path_ct path_new_parent(void)
{
    return error_pass_ptr(path_new_special(PATH_COMP_PARENT));
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
    npath->comp = NULL;
    
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

bool path_is_empty(path_const_ct path)
{
    assert_magic(path);
    
    return !path->comp || vec_is_empty(path->comp);
}

bool path_is_absolute(path_const_ct path)
{
    assert_magic(path);
    
    return path->absolute;
}

bool path_is_trailing(path_const_ct path)
{
    assert_magic(path);
    
    return path->trailing;
}

static inline bool path_check_prop(const path_prop_st *prop)
{
    return prop && prop->sep && prop->sep[0];
}

bool path_is_equal(path_const_ct path1, path_const_ct path2, const path_prop_st *prop)
{
    size_t c, comps;
    path_comp_st *comp1, *comp2;
    
    assert_magic(path1);
    assert_magic(path2);
    assert(path_check_prop(prop));
    
    return_value_if_fail(path1->absolute == path2->absolute, false);
    return_value_if_fail(path1->trailing == path2->trailing, false);
    
    comps = vec_size(path1->comp);
    return_value_if_fail(comps == vec_size(path2->comp), false);
    
    for(c=0; c < comps; c++)
    {
        comp1 = vec_at(path1->comp, c);
        comp2 = vec_at(path2->comp, c);
        
        return_value_if_fail(comp1->len == comp2->len, false);
        
        if(!comp1->name && !comp2->name)
            continue;
        
        return_value_if_fail(!comp1->name || !comp2->name, false);
        
        if(prop->case_sensitive)
            return_value_if_pass(strcmp(comp1->name, comp2->name), false);
        else
            return_value_if_pass(strcasecmp(comp1->name, comp2->name), false);
    }
    
    return true;
}

size_t path_depth(path_const_ct path)
{
    assert_magic(path);
    
    return path->comp ? vec_size(path->comp) : 0;
}

ssize_t path_len(path_const_ct path, const path_prop_st *prop)
{
    path_comp_st *comp;
    size_t c, len;
    
    assert_magic(path);
    return_error_if_fail(path_check_prop(prop), E_PATH_INVALID_PROPERTIES, -1);
    
    len = path->absolute ? 1 : 0;
    
    return_value_if_pass(!path->comp || vec_is_empty(path->comp), len);
    
    for(c=0; (comp = vec_at(path->comp, c)); c++)
    {
        len += c ? 1 : 0;
        
        if(comp->name)
        {
            len += comp->len;
        }
        else switch(comp->len)
        {
        case PATH_COMP_CURRENT:
            return_error_if_fail(prop->current, E_PATH_UNSUPPORTED, -1);
            len += strlen(prop->current);
            break;
        case PATH_COMP_PARENT:
            return_error_if_fail(prop->parent, E_PATH_UNSUPPORTED, -1);
            len += strlen(prop->parent);
            break;
        default:
            abort();
        }
    }
    
    len += path->trailing ? 1 : 0;
    
    return len;
}

ssize_t path_len_nonempty(path_const_ct path, const path_prop_st *prop)
{
    assert_magic(path);
    
    return_value_if_pass(!path->comp || vec_is_empty(path->comp), 0);
    
    return error_pass_int(path_len(path, prop));
}

static path_comp_st *path_add_comp(path_ct path, const char *str, size_t len, const path_prop_st *prop)
{
    path_comp_st *comp;
    
    if(!(comp = vec_push(path->comp)))
        return error_wrap(), NULL;
    
    if(prop->current && len == strlen(prop->current) && !strncmp(str, prop->current, len))
        return comp->len = PATH_COMP_CURRENT, comp;
    
    if(prop->parent && len == strlen(prop->parent) && !strncmp(str, prop->parent, len))
        return comp->len = PATH_COMP_PARENT, comp;
    
    if(!(comp->name = strndup(str, len)))
        return error_wrap_errno(strndup), vec_pop(path->comp), NULL;
    
    comp->len = len;
    
    return comp;
}

static path_ct path_parse_comp(path_ct path, const char *str, size_t len, const path_prop_st *prop)
{
    const char *ptr;
    
    if(!str || !len)
        return path;
    
    // initial path separator
    path->absolute = !!strchr(prop->sep, str[0]);
    
    if(!path->comp && !(path->comp = vec_new(AVG_PATH_COMPS, sizeof(path_comp_st))))
        return error_wrap(), NULL;
    
    for(; (ptr = strnpbrk(str, prop->sep, len)); len -= ptr-str+1, str = ptr+1)
    {
        if(ptr == str)
            continue; // skip repeating separator
        
        if(!path_add_comp(path, str, ptr - str, prop))
            return error_pass(), NULL;
    }
    
    // trailing path separator
    // absolute and trailing are both true even if str has only separators
    path->trailing = !len;
    
    if(len && !path_add_comp(path, str, len, prop))
        return error_pass(), NULL;
    
    return path;
}

path_ct path_set(path_ct path, str_const_ct str, const path_prop_st *prop)
{
    return error_pass_ptr(path_set_cn(path, str_bc(str), str_len(str), prop));
}

path_ct path_set_c(path_ct path, const char *str, const path_prop_st *prop)
{
    return error_pass_ptr(path_set_cn(path, str, strlen(str), prop));
}

path_ct path_set_cn(path_ct path, const char *str, size_t len, const path_prop_st *prop)
{
    path_st npath = {0};
    
    assert_magic(path);
    return_error_if_fail(path_check_prop(prop), E_PATH_INVALID_PROPERTIES, NULL);
    
    init_magic(&npath);
    
    if(!path_parse_comp(&npath, str, len, prop))
        return error_pass(), path_clear(&npath), NULL;
    
    path_clear(path);
    memcpy(path, &npath, sizeof(path_st));
    
    return path;
}

path_ct path_append(path_ct path, str_const_ct str, const path_prop_st *prop)
{
    return error_pass_ptr(path_append_cn(path, str_bc(str), str_len(str), prop));
}

path_ct path_append_c(path_ct path, const char *str, const path_prop_st *prop)
{
    return error_pass_ptr(path_append_cn(path, str, strlen(str), prop));
}

path_ct path_append_cn(path_ct path, const char *str, size_t len, const path_prop_st *prop)
{
    assert_magic(path);
    return_error_if_fail(path_check_prop(prop), E_PATH_INVALID_PROPERTIES, NULL);
    
    return error_pass_ptr(path_parse_comp(path, str, len, prop));
}

void path_set_absolute(path_ct path, bool absolute)
{
    assert_magic(path);
    
    path->absolute = absolute;
}

void path_set_trailing(path_ct path, bool trailing)
{
    assert_magic(path);
    
    path->trailing = trailing;
}

path_ct path_drop(path_ct path, size_t n)
{
    assert_magic(path);
    
    if(path->comp)
        vec_pop_fn(path->comp, n, path_vec_free_comp, NULL);
    
    return path;
}

str_ct path_get(path_const_ct path, const path_prop_st *prop)
{
    path_comp_st *comp;
    size_t c, comps;
    str_ct str;
    
    assert_magic(path);
    return_error_if_fail(path_check_prop(prop), E_PATH_INVALID_PROPERTIES, NULL);
    
    if(!(str = path->absolute ? str_prepare_set(1, prop->sep[0]) : str_new_l("")))
        return error_wrap(), NULL;
    
    comps = path->comp ? vec_size(path->comp) : 0;
    
    for(c=0; c < comps; c++)
    {
        comp = vec_at(path->comp, c);
        
        if(c && !str_append_set(str, 1, prop->sep[0]))
            return error_wrap(), str_unref(str), NULL;
        
        if(comp->name)
        {
            if(!str_append_cn(str, comp->name, comp->len))
                return error_wrap(), str_unref(str), NULL;
        }
        else switch(comp->len)
        {
        case PATH_COMP_CURRENT:
            if(!prop->current)
                return error_set(E_PATH_UNSUPPORTED), str_unref(str), NULL;
            if(!str_append_c(str, prop->current))
                return error_wrap(), str_unref(str), NULL;
            break;
        case PATH_COMP_PARENT:
            if(!prop->parent)
                return error_set(E_PATH_UNSUPPORTED), str_unref(str), NULL;
            if(!str_append_c(str, prop->parent))
                return error_wrap(), str_unref(str), NULL;
            break;
        default:
            abort();
        }
    }
    
    // only append trailing separator if path has components
    if(comps && path->trailing && !str_append_set(str, 1, prop->sep[0]))
        return error_wrap(), str_unref(str), NULL;
    
    return str;
}

str_ct path_get_nonempty(path_const_ct path, const path_prop_st *prop)
{
    assert_magic(path);
    
    if(!path->comp || vec_is_empty(path->comp))
        return error_wrap_ptr(str_new_l(""));
    else
        return error_pass_ptr(path_get(path, prop));
}
