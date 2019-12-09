/*
 * Copyright (c) 2017-2019 Martin Rödel aka Yomin
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

#ifndef __YTIL_CON_ART_H__
#define __YTIL_CON_ART_H__

#include <stddef.h>
#include <stdbool.h>
#include <ytil/gen/str.h>
#include <ytil/cast.h>


typedef enum art_error
{
      E_ART_CALLBACK
    , E_ART_EMPTY
    , E_ART_EXISTS
    , E_ART_INVALID_KEY
    , E_ART_NOT_FOUND
} art_error_id;

typedef enum art_mode
{
      ART_MODE_ORDERED
    , ART_MODE_UNORDERED
} art_mode_id;

struct art;
struct art_node;

typedef       struct art      *art_ct;
typedef const struct art      *art_const_ct;
typedef       struct art_node *art_node_ct;
typedef const struct art_node *art_node_const_ct;

// free ART leaf node data
typedef void   (*art_dtor_cb)(art_const_ct art, void *data, void *ctx);
// return size of ART leaf node data
typedef size_t (*art_size_cb)(art_const_ct art, void *data, void *ctx);
// return true if pred matches ART leaf node
// key is NULL if not reconstructed
typedef bool   (*art_pred_cb)(art_const_ct art, str_const_ct key, const void *data, void *ctx);
// return 0 to continue fold, anything else stops fold
// key is NULL if not reconstructed
typedef int    (*art_fold_cb)(art_const_ct art, str_const_ct key, void *data, void *ctx);


// create new ART
art_ct art_new(art_mode_id mode);
// free ART
void   art_free(art_ct art);
// free ART, apply dtor on each leaf node
void   art_free_f(art_ct art, art_dtor_cb dtor, void *ctx);
// free ART if empty
art_ct art_free_if_empty(art_ct art);

// remove all nodes from ART
void art_clear(art_ct art);
// remove all nodes from ART, apply dtor on each leaf node
void art_clear_f(art_ct art, art_dtor_cb dtor, void *ctx);

// check if ART is empty
bool   art_is_empty(art_const_ct art);
// get ART leaf node count
size_t art_size(art_const_ct art);
// get allocated size of ART
size_t art_memsize(art_const_ct art);
// get allocated size of ART, apply sfun on each leaf node to determine size
size_t art_memsize_f(art_const_ct art, art_size_cb sfun, void *ctx);

// get ART leaf node data
void   *art_node_get_data(art_node_const_ct node);
#define art_node_get_value(node, type) POINTER_TO_VALUE(art_node_get_data(node), type)
// set ART leaf node data
void    art_node_set_data(art_node_ct node, const void *data);
#define art_node_set_value(node, value) art_node_set_data(node, VALUE_TO_POINTER(value))
// reconstruct ART node key
str_ct  art_node_get_key(art_node_const_ct node);

// get ART leaf node with key
art_node_ct art_get(art_const_ct art, str_const_ct key);
// get data from ART leaf node with key
void       *art_get_data(art_const_ct art, str_const_ct key);
#define     art_get_value(art, key, type) POINTER_TO_VALUE(art_get_data(art, key), type)

// insert data with key into ART
art_node_ct art_insert(art_ct art, str_const_ct key, void *data);
#define     art_insert_value(art, key, value) art_insert(art, key, VALUE_TO_POINTER(value))

// remove leaf node from ART
void art_remove(art_ct art, art_node_ct node);
// remove all nodes from ART with matching prefix
int  art_remove_p(art_ct art, str_const_ct prefix);
// apply dtor and remove all nodes from ART with matching prefix
int  art_remove_pf(art_ct art, str_const_ct prefix, art_dtor_cb dtor, void *ctx);

// find first ART leaf node matching pred
art_node_ct art_find(art_const_ct art, art_pred_cb pred, void *ctx);
void       *art_find_data(art_const_ct art, art_pred_cb pred, void *ctx);
#define     art_find_value(art, pred, ctx, type) \
            POINTER_TO_VALUE(art_find_data(art, pred, ctx), type)
// find first ART leaf node matching pred, reconstruct node key
art_node_ct art_find_k(art_const_ct art, art_pred_cb pred, void *ctx);
void       *art_find_data_k(art_const_ct art, art_pred_cb pred, void *ctx);
#define     art_find_value_k(art, pred, ctx, type) \
            POINTER_TO_VALUE(art_find_data_k(art, pred, ctx), type)
// find last ART leaf node matching pred
art_node_ct art_find_r(art_const_ct art, art_pred_cb pred, void *ctx);
void       *art_find_data_r(art_const_ct art, art_pred_cb pred, void *ctx);
#define     art_find_value_r(art, pred, ctx, type) \
            POINTER_TO_VALUE(art_find_data_r(art, pred, ctx), type)
// find last ART leaf node matching pred, reconstruct node key
art_node_ct art_find_rk(art_const_ct art, art_pred_cb pred, void *ctx);
void       *art_find_data_rk(art_const_ct art, art_pred_cb pred, void *ctx);
#define     art_find_value_rk(art, pred, ctx, type) \
            POINTER_TO_VALUE(art_find_data_rk(art, pred, ctx), type)

// find first ART leaf node with prefix matching pred
art_node_ct art_find_p(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx);
void       *art_find_data_p(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx);
#define     art_find_value_p(art, prefix, pred, ctx, type) \
            POINTER_TO_VALUE(art_find_data_p(art, prefix, pred, ctx), type)
// find first ART leaf node with prefix matching pred, reconstruct node key
art_node_ct art_find_pk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx);
void       *art_find_data_pk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx);
#define     art_find_value_pk(art, prefix, pred, ctx, type) \
            POINTER_TO_VALUE(art_find_data_pk(art, prefix, pred, ctx), type)
// find last ART leaf node with prefix matching pred
art_node_ct art_find_pr(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx);
void       *art_find_data_pr(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx);
#define     art_find_value_pr(art, prefix, pred, ctx, type) \
            POINTER_TO_VALUE(art_find_data_pr(art, prefix, pred, ctx), type)
// find last ART leaf node with prefix matching pred, reconstruct node key
art_node_ct art_find_prk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx);
void       *art_find_data_prk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx);
#define     art_find_value_prk(art, prefix, pred, ctx, type) \
            POINTER_TO_VALUE(art_find_data_prk(art, prefix, pred, ctx), type)

// apply fold to each leaf node in ART, begin with smallest key
int art_fold(art_ct art, art_fold_cb fold, void *ctx);
// apply fold to each leaf node in ART, begin with smallest key, reconstruct node key
int art_fold_k(art_ct art, art_fold_cb fold, void *ctx);
// apply fold to each leaf node in ART, begin with largest key
int art_fold_r(art_ct art, art_fold_cb fold, void *ctx);
// apply fold to each leaf node in ART, begin with largest key, reconstruct node key
int art_fold_rk(art_ct art, art_fold_cb fold, void *ctx);

// apply fold to each leaf node with prefix in ART, begin with smallest key
int art_fold_p(art_ct art, str_const_ct prefix, art_fold_cb fold, void *ctx);
// apply fold to each leaf node with prefix in ART, begin with smallest key
// reconstruct node key
int art_fold_pk(art_ct art, str_const_ct prefix, art_fold_cb fold, void *ctx);
// apply fold to each leaf node with prefix in ART, begin with largest key
int art_fold_pr(art_ct art, str_const_ct prefix, art_fold_cb fold, void *ctx);
// apply fold to each leaf node with prefix in ART, begin with largest key
// reconstruct node key
int art_fold_prk(art_ct art, str_const_ct prefix, art_fold_cb fold, void *ctx);

// get common prefix path of all leaf nodes with prefix
str_ct art_complete(art_const_ct art, str_const_ct prefix);

#endif
