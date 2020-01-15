/*
 * Copyright (c) 2018-2019 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef __YTIL_CON_LIST_H__
#define __YTIL_CON_LIST_H__

#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <ytil/cast.h>


typedef enum list_error
{
      E_LIST_CALLBACK
    , E_LIST_EMPTY
    , E_LIST_END
    , E_LIST_NOT_FOUND
    , E_LIST_OUT_OF_BOUNDS
} list_error_id;

struct list;
struct list_node;

typedef       struct list      *list_ct;
typedef const struct list      *list_const_ct;
typedef       struct list_node *list_node_ct;
typedef const struct list_node *list_node_const_ct;

// return size of list node data
typedef size_t (*list_size_cb)(list_const_ct list, const void *data, void *ctx);
// free list node data
typedef void   (*list_dtor_cb)(list_const_ct list, void *data, void *ctx);
// return true if pref matches list node data
typedef bool   (*list_pred_cb)(list_const_ct list, const void *data, void *ctx);
// return 0 to continue fold, anything else stops fold
typedef int    (*list_fold_cb)(list_const_ct list, void *data, void *ctx);
// duplicate src list node data into dst, return 0 on success
typedef int    (*list_clone_cb)(list_const_ct src_list, void **dst, const void *src, void *ctx);


// create new list
list_ct list_new(void);
// free list
void    list_free(list_ct list);
// free list, apply dtor on each list node data
void    list_free_f(list_ct list, list_dtor_cb dtor, void *ctx);
// free list if empty
list_ct list_free_if_empty(list_ct list);

// remove all list nodes
void list_clear(list_ct list);
// remove all list nodes, apply dtor on each list node data
void list_clear_f(list_ct list, list_dtor_cb dtor, void *ctx);

// duplicate list
list_ct list_clone(list_const_ct list);
// duplicate list, duplicate list node data with clone function
// in case the clone fails, already duplicated nodes are freed with dtor
list_ct list_clone_f(list_const_ct list, list_clone_cb clone, list_dtor_cb dtor, void *ctx);

// check if list is empty
bool   list_is_empty(list_const_ct list);
// get lits node count
size_t list_size(list_const_ct list);
// get allocated size of list
size_t list_memsize(list_const_ct list);
// get allocated size of list, apply sfun on each node data to determine size
size_t list_memsize_f(list_const_ct list, list_size_cb sfun, void *ctx);

// get list node data
void   *list_node_get_data(list_node_const_ct node);
#define list_node_get_value(node, type) POINTER_TO_VALUE(list_node_get_data(node), type)
// set list node data
void    list_node_set_data(list_node_ct node, void *data);
#define list_node_set_value(node, value) list_node_set_data(node, VALUE_TO_POINTER(value))

// get list node at pos, negative pos counts from last node
list_node_ct list_at(list_const_ct list, ssize_t pos);
// get first list node
list_node_ct list_first(list_const_ct list);
// get last list node
list_node_ct list_last(list_const_ct list);
// get successor list node
list_node_ct list_next(list_const_ct list, list_node_const_ct node);
// get predecessor list node
list_node_ct list_prev(list_const_ct list, list_node_const_ct node);

// get data from list node at pos, negative pos counts from last node
void   *list_data_at(list_const_ct list, ssize_t pos);
#define list_value_at(list, pos, type) POINTER_TO_VALUE(list_data_at(list, pos), type)
// get data from first list node
void   *list_data_first(list_const_ct list);
#define list_value_first(list, type) POINTER_TO_VALUE(list_data_first(list), type)
// get data from last list node
void   *list_data_last(list_const_ct list);
#define list_value_last(list, type) POINTER_TO_VALUE(list_data_last(list), type)

// get position of list node in list
ssize_t list_pos(list_const_ct list, list_node_const_ct node);

// insert node as list head
list_node_ct list_prepend(list_ct list, void *data);
#define      list_prepend_value(list, value) list_prepend(list, VALUE_TO_POINTER(value))
// insert node as list tail
list_node_ct list_append(list_ct list, void *data);
#define      list_append_value(list, value) list_append(list, VALUE_TO_POINTER(value))
// insert list node at pos, negative pos counts from last node
list_node_ct list_insert(list_ct list, ssize_t pos, void *data);
#define      list_insert_value(list, pos, value) list_insert(list, pos, VALUE_TO_POINTER(value))
// insert list node before suc into list
list_node_ct list_insert_before(list_ct list, list_node_const_ct suc, void *data);
#define      list_insert_value_before(list, suc, value) list_insert_before(list, suc, VALUE_TO_POINTER(value))
// insert list node after pre into list
list_node_ct list_insert_after(list_ct list, list_node_const_ct pre, void *data);
#define      list_insert_value_after(list, pre, value) list_insert_after(list, pre, VALUE_TO_POINTER(value))

// remove node from list
void list_remove(list_ct list, list_node_ct node);
// remove node at pos from list, negative pos counts from last node
int  list_remove_at(list_ct list, ssize_t pos);
// apply dtor to node at pos and remove it from list, negative pos counts from last node
int  list_remove_at_f(list_ct list, ssize_t pos, list_dtor_cb dtor, void *ctx);

// find first node in list matching pred
list_node_ct list_find(list_const_ct list, list_pred_cb pred, void *ctx);
// find last node in list matching pred
list_node_ct list_find_r(list_const_ct list, list_pred_cb pred, void *ctx);
// find first node in list matching pred and remove it
int          list_find_remove(list_ct list, list_pred_cb pred, void *ctx);
// find first node in list matching pred, apply dtor and remove it
int          list_find_remove_f(list_ct list, list_pred_cb pred, void *pred_ctx, list_dtor_cb dtor, void *dtor_ctx);
// find last node in list matching pred and remove it
int          list_find_remove_r(list_ct list, list_pred_cb pred, void *ctx);
// find last node in list matching pred, apply dtor and remove it
int          list_find_remove_rf(list_ct list, list_pred_cb pred, void *pred_ctx, list_dtor_cb dtor, void *dtor_ctx);
// find all nodes in list matching pred and remove them
size_t       list_find_remove_all(list_ct list, list_pred_cb pred, void *ctx);
// find all nodes in list matching pred, apply dtor and remove them
size_t       list_find_remove_all_f(list_ct list, list_pred_cb pred, void *pred_ctx, list_dtor_cb dtor, void *dtor_ctx);

// swap two nodes in list
void list_swap(list_node_const_ct node1, list_node_const_ct node2);

// apply fold to every list node, begin with first node
int list_fold(list_const_ct list, list_fold_cb fold, void *ctx);
// apply fold to every list node, begin with last node
int list_fold_r(list_const_ct list, list_fold_cb fold, void *ctx);

#endif
