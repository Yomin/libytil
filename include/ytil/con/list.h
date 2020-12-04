/*
 * Copyright (c) 2018-2020 Martin Rödel a.k.a. Yomin Nimoy
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

/// \file

#ifndef YTIL_CON_LIST_H_INCLUDED
#define YTIL_CON_LIST_H_INCLUDED

#include <ytil/def/cast.h>
#include <ytil/gen/error.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>


/// list error
typedef enum list_error
{
    E_LIST_CALLBACK,        ///< callback error
    E_LIST_EMPTY,           ///< list is empty
    E_LIST_END,             ///< list end reached
    E_LIST_NOT_FOUND,       ///< node not found
    E_LIST_OUT_OF_BOUNDS,   ///< out of bounds node access
} list_error_id;

/// list error type declaration
ERROR_DECLARE(LIST);

struct list;
struct list_node;

typedef       struct list   *list_ct;       ///< list type
typedef const struct list   *list_const_ct; ///< const list type

typedef       struct list_node  *list_node_ct;          ///< list node type
typedef const struct list_node  *list_node_const_ct;    ///< const list node type

/// list node size callback
///
/// \param list     list
/// \param data     node data to get size of
/// \param ctx      callback context
///
/// \returns        size of node
typedef size_t (*list_size_cb)(list_const_ct list, const void *data, void *ctx);

/// list node dtor callback
///
/// \param list     list
/// \param data     node data to destroy
/// \param ctx      callback context
typedef void (*list_dtor_cb)(list_const_ct list, void *data, void *ctx);

/// list node predicate callback
///
/// \param list     list
/// \param data     node data to check
/// \param ctx      callback context
///
/// \retval true    \p data matches predicate
/// \retval false   \p data does not match predicate
typedef bool (*list_pred_cb)(list_const_ct list, const void *data, void *ctx);

/// list node fold callback
///
/// \param list     list
/// \param data     node data
/// \param ctx      callback context
///
/// \retval 0       continue fold
/// \retval <0      stop fold with error
/// \retval >0      stop fold
typedef int (*list_fold_cb)(list_const_ct list, void *data, void *ctx);

/// list node clone callback
///
/// \param      src_list    source list
/// \param[out] dst         node data to fill
/// \param      src         node data to copy
/// \param      ctx         callback context
///
/// \retval 0               success
/// \retval !=0             error
typedef int (*list_clone_cb)(list_const_ct src_list, void **dst, const void *src, void *ctx);


/// Create new list.
///
/// \returns                    new list
/// \retval NULL/E_GENERIC_OOM  out of memory
list_ct list_new(void);

/// Free list.
///
/// \param list     list
void list_free(list_ct list);

/// Destroy nodes and free list.
///
/// \param list     list
/// \param dtor     callback to destroy node data, may be NULL
/// \param ctx      \p dtor context
void list_free_f(list_ct list, list_dtor_cb dtor, const void *ctx);

/// Free list if empty.
///
/// \param list     list
///
/// \retval list    list is not empty
/// \retval NULL    list was empty and was freed
list_ct list_free_if_empty(list_ct list);

/// Destroy nodes and free list if empty.
///
/// \param list     list
/// \param dtor     callback to destroy node data, may be NULL
/// \param ctx      \p dtor context
///
/// \retval list    list is not empty
/// \retval NULL    list was empty and was freed
list_ct list_free_if_empty_f(list_ct list, list_dtor_cb dtor, const void *ctx);

/// Remove all nodes.
///
/// \param list     list
void list_clear(list_ct list);

/// Destroy and remove all nodes.
///
/// \param list     list
/// \param dtor     callback to destroy node data, may be NULL
/// \param ctx      \p dtor context
void list_clear_f(list_ct list, list_dtor_cb dtor, const void *ctx);

/// Clone list.
///
/// \param list     list
///
/// \returns                        new list
/// \retval NULL/E_GENERIC_OOM      out of memory
list_ct list_clone(list_const_ct list);

/// Clone list and node data.
///
/// If clone callback is given, invoke clone on each node,
/// else memdup each node.
/// If cloning fails and dtor callback is given,
/// invoke dtor on each already cloned node.
///
/// \param list     list
/// \param clone    callback to duplicate node data, may be NULL
/// \param dtor     callback to destroy node data, may be NULL
/// \param ctx      \p clone and \p dtor context
///
/// \returns                        new list
/// \retval NULL/E_GENERIC_OOM      out of memory
/// \retval NULL/E_LIST_CALLBACK    \p clone callback error
list_ct list_clone_f(list_const_ct list, list_clone_cb clone, list_dtor_cb dtor, const void *ctx);

/// Check if list is empty.
///
/// \param list     list
///
/// \retval true    list is empty
/// \retval false   list is not empty
bool list_is_empty(list_const_ct list);

/// Get number of list nodes.
///
/// \param list     list
///
/// \returns        number of list nodes
size_t list_size(list_const_ct list);

/// Get allocated size of list.
///
/// \param list     list
///
/// \returns        allocated size in bytes
size_t list_memsize(list_const_ct list);

/// Get allocated size of list and node data.
///
/// \param list     list
/// \param size     callback to determine size of node data, may be NULL
/// \param ctx      \p size context
///
/// \returns        allocated size in bytes
size_t list_memsize_f(list_const_ct list, list_size_cb size, const void *ctx);

/// Get list node data.
///
/// \param node     node to get data from
///
/// \returns        node data
void *list_node_data(list_node_const_ct node);

/// Get list node value.
///
/// \param node     node to get value from
/// \param type     type to cast data pointer to
///
/// \returns        node data pointer casted to \p type
#define list_node_value(node, type) \
    POINTER_TO_VALUE(list_node_data(node), type)

/// Set list node data.
///
/// \param node     node to set data to
/// \param data     data to set
void list_node_set(list_node_ct node, const void *data);

/// Set list node value.
///
/// \param node     node to set value to
/// \param value    value to set
#define list_node_set_value(node, value) \
    list_node_set(node, VALUE_TO_POINTER(value))

/// Get list node at position.
///
/// \param list     list
/// \param pos      position of node, negative value counts from last node
///
/// \returns                            list node
/// \retval NULL/E_LIST_OUT_OF_BOUNDS   \p pos is out of bounds
list_node_ct list_at(list_const_ct list, ssize_t pos);

/// Get first list node.
///
/// \param list     list
///
/// \returns                    list node
/// \retval NULL/E_LIST_EMPTY   list is empty
list_node_ct list_first(list_const_ct list);

/// Get last list node.
///
/// \param list     list
///
/// \returns                    list node
/// \retval NULL/E_LIST_EMPTY   list is empty
list_node_ct list_last(list_const_ct list);

/// Get next list node.
///
/// \param list     list
/// \param node     node to get successor from
///
/// \returns                    list node
/// \retval NULL/E_LIST_END     \p node is last list node
list_node_ct list_next(list_const_ct list, list_node_const_ct node);

/// Get previous list node.
///
/// \param list     list
/// \param node     node to get predecessor from
///
/// \returns                    list node
/// \retval NULL/E_LIST_END     \p node is first list node
list_node_ct list_prev(list_const_ct list, list_node_const_ct node);

/// Get list node data at position.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param list     list
/// \param pos      position of node, negative value counts from last node
///
/// \returns                            list node data
/// \retval NULL/E_LIST_OUT_OF_BOUNDS   \p pos is out of bounds
void *list_data_at(list_const_ct list, ssize_t pos);

/// Get list node value at position.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param list     list
/// \param pos      position of node, negative value counts from last node
/// \param type     type to cast data pointer to
///
/// \returns                            node data pointer casted to \p type
/// \retval NULL/E_LIST_OUT_OF_BOUNDS   \p pos is out of bounds
#define list_value_at(list, pos, type) \
    POINTER_TO_VALUE(list_data_at(list, pos), type)

/// Get first list node data.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param list     list
///
/// \returns                    list node data
/// \retval NULL/E_LIST_EMPTY   list is empty
void *list_data_first(list_const_ct list);

/// Get first list node value.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param list     list
/// \param type     type to cast data pointer to
///
/// \returns                    node data pointer casted to \p type
/// \retval NULL/E_LIST_EMPTY   list is empty
#define list_value_first(list, type) \
    POINTER_TO_VALUE(list_data_first(list), type)

/// Get last list node data.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param list     list
///
/// \returns                    list node data
/// \retval NULL/E_LIST_EMPTY   list is empty
void *list_data_last(list_const_ct list);

/// Get last list node value.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param list     list
/// \param type     type to cast data pointer to
///
/// \returns                    node data pointer casted to \p type
/// \retval NULL/E_LIST_EMPTY   list is empty
#define list_value_last(list, type) \
    POINTER_TO_VALUE(list_data_last(list), type)

/// Get position of node in list.
///
/// \param list     list
/// \param node     node
///
/// \returns        node position
size_t list_pos(list_const_ct list, list_node_const_ct node);

/// Prepend data to list.
///
/// \param list     list
/// \param data     data to set node with
///
/// \returns                    list node
/// \retval NULL/E_GENERIC_OOM  out of memory
list_node_ct list_prepend(list_ct list, const void *data);

/// Prepend value to list.
///
/// \param list     list
/// \param value    value to set node with
///
/// \returns                    list node
/// \retval NULL/E_GENERIC_OOM  out of memory
#define list_prepend_value(list, value) \
    list_prepend(list, VALUE_TO_POINTER(value))

/// Append data to list.
///
/// \param list     list
/// \param data     data to set node with
///
/// \returns                    list node
/// \retval NULL/E_GENERIC_OOM  out of memory
list_node_ct list_append(list_ct list, const void *data);

/// Append value to list.
///
/// \param list     list
/// \param value    value to set node with
///
/// \returns                    list node
/// \retval NULL/E_GENERIC_OOM  out of memory
#define list_append_value(list, value) \
    list_append(list, VALUE_TO_POINTER(value))

/// Insert data into list at position.
///
/// \param list     list
/// \param pos      insert position, negative value counts from last node
/// \param data     data to set node with
///
/// \returns                            list node
/// \retval NULL/E_LIST_OUT_OF_BOUNDS   \p pos is out of bounds
/// \retval NULL/E_GENERIC_OOM          out of memory
list_node_ct list_insert(list_ct list, ssize_t pos, const void *data);

/// Insert value into list at position.
///
/// \param list     list
/// \param pos      insert position, negative value counts from last node
/// \param value    value to set node with
///
/// \returns                            list node
/// \retval NULL/E_LIST_OUT_OF_BOUNDS   \p pos is out of bounds
/// \retval NULL/E_GENERIC_OOM          out of memory
#define list_insert_value(list, pos, value) \
    list_insert(list, pos, VALUE_TO_POINTER(value))

/// Insert data before node into list.
///
/// \param list     list
/// \param suc      node to insert before
/// \param data     data to set node with
///
/// \returns                            list node
/// \retval NULL/E_GENERIC_OOM          out of memory
list_node_ct list_insert_before(list_ct list, list_node_const_ct suc, const void *data);

/// Insert value before node into list.
///
/// \param list     list
/// \param suc      node to insert before
/// \param value    value to set node with
///
/// \returns                            list node
/// \retval NULL/E_GENERIC_OOM          out of memory
#define list_insert_value_before(list, suc, value) \
    list_insert_before(list, suc, VALUE_TO_POINTER(value))

/// Insert data after node into list.
///
/// \param list     list
/// \param pre      node to insert after
/// \param data     data to set node with
///
/// \returns                            list node
/// \retval NULL/E_GENERIC_OOM          out of memory
list_node_ct list_insert_after(list_ct list, list_node_const_ct pre, const void *data);

/// Insert value after node into list.
///
/// \param list     list
/// \param pre      node to insert after
/// \param value    value to set node with
///
/// \returns                            list node
/// \retval NULL/E_GENERIC_OOM          out of memory
#define list_insert_value_after(list, pre, value) \
    list_insert_after(list, pre, VALUE_TO_POINTER(value))

/// Remove node from list.
///
/// \param list     list
/// \param node     node to remove
void list_remove(list_ct list, list_node_ct node);

/// Remove node from list at position.
///
/// \param list     list
/// \param pos      node position, negative value counts from last node
///
/// \retval 0                           success
/// \retval NULL/E_LIST_OUT_OF_BOUNDS   \p pos is out of bounds
int list_remove_at(list_ct list, ssize_t pos);

/// Destroy and remove node from list at position.
///
/// \param list     list
/// \param pos      node position, negative value counts from last node
/// \param dtor     callback to destroy node data, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0                           success
/// \retval NULL/E_LIST_OUT_OF_BOUNDS   \p pos is out of bounds
int list_remove_at_f(list_ct list, ssize_t pos, list_dtor_cb dtor, const void *ctx);

/// Find first node matching predicate.
///
/// \param list     list
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        list node
/// \retval NULL/E_LIST_NOT_FOUND   \p pred did not match any node
list_node_ct list_find(list_const_ct list, list_pred_cb pred, const void *ctx);

/// Find last node matching predicate.
///
/// \param list     list
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        list node
/// \retval NULL/E_LIST_NOT_FOUND   \p pred did not match any node
list_node_ct list_find_r(list_const_ct list, list_pred_cb pred, const void *ctx);

/// Find and remove first node matching predicate.
///
/// \param list     list
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns 0                      success
/// \retval -1/E_LIST_NOT_FOUND     \p pred did not match any node
int list_find_remove(list_ct list, list_pred_cb pred, const void *ctx);

/// Find, destroy and remove first node matching predicate.
///
/// \param list         list
/// \param pred         predicate
/// \param pred_ctx     \p pred context
/// \param dtor         callback to destroy node data, may be NULL
/// \param dtor_ctx     \p dtor context
///
/// \returns 0                      success
/// \retval -1/E_LIST_NOT_FOUND     \p pred did not match any node
int list_find_remove_f(list_ct list, list_pred_cb pred, const void *pred_ctx, list_dtor_cb dtor, const void *dtor_ctx);

/// Find and remove last node matching predicate.
///
/// \param list     list
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns 0                      success
/// \retval -1/E_LIST_NOT_FOUND     \p pred did not match any node
int list_find_remove_r(list_ct list, list_pred_cb pred, const void *ctx);

/// Find, destroy and remove last node matching.
///
/// \param list         list
/// \param pred         predicate
/// \param pred_ctx     \p pred context
/// \param dtor         callback to destroy node data, may be NULL
/// \param dtor_ctx     \p dtor context
///
/// \returns 0                      success
/// \retval -1/E_LIST_NOT_FOUND     \p pred did not match any node
int list_find_remove_rf(list_ct list, list_pred_cb pred, const void *pred_ctx, list_dtor_cb dtor, const void *dtor_ctx);

/// Find and remove all nodes matching predicate.
///
/// \param list     list
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns        number of nodes removed
size_t list_find_remove_all(list_ct list, list_pred_cb pred, const void *ctx);

/// Find, destroy and remove all nodes matching predicate.
///
/// \param list         list
/// \param pred         predicate
/// \param pred_ctx     \p pred context
/// \param dtor         callback to destroy node data, may be NULL
/// \param dtor_ctx     \p dtor context
///
/// \returns            number of nodes removed
size_t list_find_remove_all_f(list_ct list, list_pred_cb pred, const void *pred_ctx, list_dtor_cb dtor, const void *dtor_ctx);

/// Swap positions of two nodes.
///
/// \param node1    first node
/// \param node2    second node
void list_swap(list_node_const_ct node1, list_node_const_ct node2);

/// Fold over all nodes in list, starting with first node.
///
/// \param list     list
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval <0/E_LIST_CALLBACK  \p fold error
/// \retval >0                  \p fold rc
int list_fold(list_const_ct list, list_fold_cb fold, const void *ctx);

/// Fold over all nodes in list, starting with last node.
///
/// \param list     list
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval <0/E_LIST_CALLBACK  \p fold error
/// \retval >0                  \p fold rc
int list_fold_r(list_const_ct list, list_fold_cb fold, const void *ctx);


#endif // ifndef YTIL_CON_LIST_H_INCLUDED
