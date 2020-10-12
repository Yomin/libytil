/*
 * Copyright (c) 2017-2020 Martin Rödel aka Yomin
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

#ifndef YTIL_CON_ART_H_INCLUDED
#define YTIL_CON_ART_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>
#include <ytil/gen/error.h>
#include <ytil/gen/str.h>
#include <ytil/cast.h>


/// ART error
typedef enum art_error
{
    E_ART_CALLBACK,     ///< callback error
    E_ART_EMPTY,        ///< ART is empty
    E_ART_EXISTS,       ///< node already exists
    E_ART_INVALID_KEY,  ///< invalid key
    E_ART_NOT_FOUND,    ///< node not found
} art_error_id;

/// ART error type declaration
ERROR_DECLARE(ART);

/// ART sort mode
///
/// If the architecture does not support intrinsics,
/// all children are kept in ascending order to enable binary search.
/// If intrinsics for 8, 16 or 32 bytes are available,
/// the respective children lists do not need to be in order.
/// For these cases the sort mode specifies whether to keep them sorted or not.
/// Keeping them unsorted will result in faster insert and remove operations
/// but will also affect the fold order.
typedef enum art_mode
{
    ART_MODE_ORDERED,   ///< nodes are inserted in ascending order
    ART_MODE_UNORDERED, ///< nodes are appended
} art_mode_id;

struct art;
struct art_node;

typedef       struct art    *art_ct;        ///< ART type
typedef const struct art    *art_const_ct;  ///< const ART type

typedef       struct art_node   *art_node_ct;       ///< ART node type
typedef const struct art_node   *art_node_const_ct; ///< const ART node type

/// ART node size callback
///
/// \param art      ART
/// \param data     node data to get size of
/// \param ctx      callback context
///
/// \returns        size of node
typedef size_t (*art_size_cb)(art_const_ct art, const void *data, void *ctx);

/// ART node dtor callback
///
/// \param art      ART
/// \param data     node data to destroy
/// \param ctx      callback context
typedef void (*art_dtor_cb)(art_const_ct art, void *data, void *ctx);

/// ART node predicate callback
///
/// \param art      ART
/// \param key      node key, NULL if not reconstructed
/// \param data     node data to check
/// \param ctx      callback context
///
/// \retval true    \p key and \p data match predicate
/// \retval false   \p key and \p data do not match predicate
typedef bool (*art_pred_cb)(art_const_ct art, str_const_ct key, const void *data, void *ctx);

/// ART node fold callback
///
/// \param art      ART
/// \param key      node key, NULL if not reconstructed
/// \param data     node data
/// \param ctx      callback context
///
/// \retval 0       continue fold
/// \retval <0      stop fold with error
/// \retval >0      stop fold
typedef int (*art_fold_cb)(art_const_ct art, str_const_ct key, void *data, void *ctx);


/// Create new ART.
///
/// \param mode     sort mode
///
/// \returns                    new ART
/// \retval NULL/E_GENERIC_OOM  out of memory
art_ct art_new(art_mode_id mode);

/// Free ART.
///
/// \param art      ART
void art_free(art_ct art);

/// Destroy nodes and free ART.
///
/// \param art      ART
/// \param dtor     callback to destroy node, may be NULL
/// \param ctx      \p dtor context
void art_free_f(art_ct art, art_dtor_cb dtor, const void *ctx);

/// Free ART if empty.
///
/// \param art      ART
///
/// \retval art     ART is not empty
/// \retval NULL    ART was empty and was freed
art_ct art_free_if_empty(art_ct art);

/// Destroy nodes and free ART if empty.
///
/// \param art      ART
/// \param dtor     callback to destroy node, may be NULL
/// \param ctx      \p dtor context
///
/// \retval art     ART is not empty
/// \retval NULL    ART was empty and was freed
art_ct art_free_if_empty_f(art_ct art, art_dtor_cb dtor, const void *ctx);

/// Remove all nodes.
///
/// \param art      ART
void art_clear(art_ct art);

/// Destroy and remove all nodes.
///
/// \param art      ART
/// \param dtor     callback to destroy node, may be NULL
/// \param ctx      \p dtor context
void art_clear_f(art_ct art, art_dtor_cb dtor, const void *ctx);

/// Check if ART is empty.
///
/// \param art      ART
///
/// \retval true    ART is empty
/// \retval false   ART is not empty
bool art_is_empty(art_const_ct art);

/// Get number of leaf nodes.
///
/// \param art      ART
///
/// \returns        number of leaf nodes
size_t art_size(art_const_ct art);

/// Get allocated size of ART.
///
/// \param art      ART
///
/// \returns        allocated size in bytes
size_t art_memsize(art_const_ct art);

/// Get allocated size of ART and nodes.
///
/// \param art      ART
/// \param size     callback to determine size of node data, may be NULL
/// \param ctx      \p size context
///
/// \returns        allocated size in bytes
size_t art_memsize_f(art_const_ct art, art_size_cb size, const void *ctx);

/// Reconstruct node key.
///
/// \param node     node to reconstruct key for
///
/// \returns                    node key
/// \retval NULL/E_GENERIC_OOM  out of memory
str_ct art_node_key(art_node_const_ct node);

/// Get node data.
///
/// \param node     node to get data from
///
/// \returns        node data
void *art_node_data(art_node_const_ct node);

/// Get node value.
///
/// \param node     node to get value from
/// \param type     type to cast data pointer to
///
/// \returns        node data pointer casted to \p type
#define art_node_value(node, type) \
    POINTER_TO_VALUE(art_node_data(node), type)

/// Set node data.
///
/// \param node     node to set
/// \param data     data to set
void art_node_set(art_node_ct node, const void *data);

/// Set node value.
///
/// \param node     node to set
/// \param value    value to set
#define art_node_set_value(node, value) \
    art_node_set(node, VALUE_TO_POINTER(value))

/// Get node.
///
/// \param art      ART
/// \param key      node key
///
/// \returns                        node
/// \retval NULL/E_ART_NOT_FOUND    node not found
art_node_ct art_get(art_const_ct art, str_const_ct key);

/// Get node data.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param key      node key
///
/// \returns                        node data
/// \retval NULL/E_ART_NOT_FOUND    node not found
void *art_get_data(art_const_ct art, str_const_ct key);

/// Get node value.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param key      node key
/// \param type     type to cast data pointer to
///
/// \returns                        node data pointer casted to \p type
/// \retval NULL/E_ART_NOT_FOUND    node not found
#define art_get_value(art, key, type) \
    POINTER_TO_VALUE(art_get_data(art, key), type)

/// Set or insert node.
///
/// \param art      ART
/// \param key      node key
/// \param data     data to set
///
/// \returns                        new node
/// \retval NULL/E_ART_INVALID_KEY  invalid node key
/// \retval NULL/E_GENERIC_OOM      out of memory
art_node_ct art_set(art_ct art, str_const_ct key, const void *data);

/// Set or insert node with value.
///
/// \param art      ART
/// \param key      node key
/// \param value    value to set
///
/// \returns                        new node
/// \retval NULL/E_ART_INVALID_KEY  invalid node key
/// \retval NULL/E_ART_EXISTS       node with \p key already existing
/// \retval NULL/E_GENERIC_OOM      out of memory
#define art_set_value(art, key, value) \
    art_set(art, key, VALUE_TO_POINTER(value))

/// Insert node.
///
/// \param art      ART
/// \param key      node key
/// \param data     data to set node with
///
/// \returns                        new node
/// \retval NULL/E_ART_INVALID_KEY  invalid node key
/// \retval NULL/E_ART_EXISTS       node with \p key already existing
/// \retval NULL/E_GENERIC_OOM      out of memory
art_node_ct art_insert(art_ct art, str_const_ct key, const void *data);

/// Insert node with value.
///
/// \param art      ART
/// \param key      node key
/// \param value    value to set node with
///
/// \returns                        new node
/// \retval NULL/E_ART_INVALID_KEY  invalid node key
/// \retval NULL/E_ART_EXISTS       node with \p key already existing
/// \retval NULL/E_GENERIC_OOM      out of memory
#define art_insert_value(art, key, value) \
    art_insert(art, key, VALUE_TO_POINTER(value))

/// Remove node.
///
/// \param art      ART
/// \param node     node to remove
void art_remove(art_ct art, art_node_ct node);

/// Remove all nodes with matching prefix.
///
/// \param art      ART
/// \param prefix   prefix of nodes to remove, may be NULL
///
/// \retval 0                   success
/// \retval -1/E_ART_NOT_FOUND  \p prefix not found
int art_remove_p(art_ct art, str_const_ct prefix);

/// Destroy and remove all nodes with matching prefix.
///
/// \param art      ART
/// \param prefix   prefix of nodes to remove, may be NULL
/// \param dtor     callback to destroy node data, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0                   success
/// \retval -1/E_ART_NOT_FOUND  \p prefix not found
int art_remove_pf(art_ct art, str_const_ct prefix, art_dtor_cb dtor, const void *ctx);

/// Find first node matching predicate.
///
/// \param art      ART
/// \param pred     predicate to apply
/// \param ctx      \p pred context
///
/// \returns                        node
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
art_node_ct art_find(art_const_ct art, art_pred_cb pred, const void *ctx);

/// Find first node matching predicate and return node data.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node data
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
void *art_find_data(art_const_ct art, art_pred_cb pred, const void *ctx);

/// Find first node matching predicate and return node value.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
/// \param type     type to cast data pointer to
///
/// \returns                        node data pointer casted to \p type
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
#define art_find_value(art, pred, ctx, type) \
    POINTER_TO_VALUE(art_find_data(art, pred, ctx), type)

/// Find first node matching predicate.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
art_node_ct art_find_k(art_const_ct art, art_pred_cb pred, const void *ctx);

/// Find first node matching predicate and return node data.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node data
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
void *art_find_data_k(art_const_ct art, art_pred_cb pred, const void *ctx);

/// Find first node matching predicate and return node value.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
/// \param type     type to cast data pointer to
///
/// \returns                        node data pointer casted to \p type
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
#define art_find_value_k(art, pred, ctx, type) \
    POINTER_TO_VALUE(art_find_data_k(art, pred, ctx), type)

/// Find last node matching predicate.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
art_node_ct art_find_r(art_const_ct art, art_pred_cb pred, const void *ctx);

/// Find last node matching predicate and return node data.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node data
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
void *art_find_data_r(art_const_ct art, art_pred_cb pred, const void *ctx);

/// Find last node matching predicate and return node value.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
/// \param type     type to cast data pointer to
///
/// \returns                        node data pointer casted to \p type
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
#define art_find_value_r(art, pred, ctx, type) \
    POINTER_TO_VALUE(art_find_data_r(art, pred, ctx), type)

/// Find last node matching predicate.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
art_node_ct art_find_rk(art_const_ct art, art_pred_cb pred, const void *ctx);

/// Find last node matching predicate and return node data.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node data
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
void *art_find_data_rk(art_const_ct art, art_pred_cb pred, const void *ctx);

/// Find last node matching predicate and return node value.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param pred     predicate
/// \param ctx      \p pred context
/// \param type     type to cast data pointer to
///
/// \returns                        node data pointer casted to \p type
/// \retval NULL/E_ART_NOT_FOUND    \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
#define art_find_value_rk(art, pred, ctx, type) \
    POINTER_TO_VALUE(art_find_data_rk(art, pred, ctx), type)

/// Match prefix and find first node matching predicate.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
art_node_ct art_find_p(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx);

/// Match prefix, find first node matching predicate and return node data.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node data
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
void *art_find_data_p(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx);

/// Match prefix, find first node matching predicate and return node value.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
/// \param type     type to cast data pointer to
///
/// \returns                        node data pointer casted to \p type
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
#define art_find_value_p(art, prefix, pred, ctx, type) \
    POINTER_TO_VALUE(art_find_data_p(art, prefix, pred, ctx), type)

/// Match prefix and find first node matching predicate.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
art_node_ct art_find_pk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx);

/// Match prefix, find first node matching predicate and return node data.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node data
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
void *art_find_data_pk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx);

/// Match prefix, find first node matching predicate and return node value.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
/// \param type     type to cast data pointer to
///
/// \returns                        node data pointer casted to \p type
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
#define art_find_value_pk(art, prefix, pred, ctx, type) \
    POINTER_TO_VALUE(art_find_data_pk(art, prefix, pred, ctx), type)

/// Match prefix and find last node matching predicate.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
art_node_ct art_find_pr(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx);

/// Match prefix, find last node matching predicate and return node data.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node data
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
void *art_find_data_pr(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx);

/// Match prefix, find last node matching predicate and return node value.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
/// \param type     type to cast data pointer to
///
/// \returns                        node data pointer casted to \p type
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
#define art_find_value_pr(art, prefix, pred, ctx, type) \
    POINTER_TO_VALUE(art_find_data_pr(art, prefix, pred, ctx), type)

/// Match prefix and find last node matching predicate.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
art_node_ct art_find_prk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx);

/// Match prefix, find last node matching predicate and return node data.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        node data
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
void *art_find_data_prk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx);

/// Match prefix, find last node matching predicate and return node value.
///
/// The node key is reconstructed and passed to \p pred callback.
///
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param pred     predicate
/// \param ctx      \p pred context
/// \param type     type to cast data pointer to
///
/// \returns                        node data pointer casted to \p type
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
/// \retval NULL/E_GENERIC_OOM      out of memory
#define art_find_value_prk(art, prefix, pred, ctx, type) \
    POINTER_TO_VALUE(art_find_data_prk(art, prefix, pred, ctx), type)

/// Fold over all nodes in ART, starting with smallest key.
///
/// \param art      ART
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval <0/E_ART_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
int art_fold(art_ct art, art_fold_cb fold, const void *ctx);

/// Fold over all nodes in ART, starting with smallest key.
///
/// The node key is reconstructed and passed to \p fold callback.
///
/// \param art      ART
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval >0                  \p fold rc
/// \retval <0/E_ART_CALLBACK   \p fold error
/// \retval -1/E_GENERIC_OOM    out of memory
int art_fold_k(art_ct art, art_fold_cb fold, const void *ctx);

/// Fold over all nodes in ART, starting with largest key.
///
/// \param art      ART
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval >0                  \p fold rc
/// \retval <0/E_ART_CALLBACK   \p fold error
int art_fold_r(art_ct art, art_fold_cb fold, const void *ctx);

/// Fold over all nodes in ART, starting with largest key.
///
/// The node key is reconstructed and passed to \p fold callback.
///
/// \param art      ART
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval >0                  \p fold rc
/// \retval <0/E_ART_CALLBACK   \p fold error
/// \retval -1/E_GENERIC_OOM    out of memory
int art_fold_rk(art_ct art, art_fold_cb fold, const void *ctx);

/// Match prefix and fold over all matched nodes, starting with smallest key.
///
/// \param art      ART
/// \param prefix   prefix of nodes to match, may be NULL
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval >0                  \p fold rc
/// \retval <0/E_ART_CALLBACK   \p fold error
/// \retval -1/E_ART_NOT_FOUND  \p prefix not found
int art_fold_p(art_ct art, str_const_ct prefix, art_fold_cb fold, const void *ctx);

/// Match prefix and fold over all matched nodes, starting with smallest key.
///
/// The node key is reconstructed and passed to \p fold callback.
///
/// \param art      ART
/// \param prefix   prefix of nodes to match, may be NULL
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval >0                  \p fold rc
/// \retval <0/E_ART_CALLBACK   \p fold error
/// \retval -1/E_ART_NOT_FOUND  \p prefix not found
/// \retval -1/E_GENERIC_OOM    out of memory
int art_fold_pk(art_ct art, str_const_ct prefix, art_fold_cb fold, const void *ctx);

/// Match prefix and fold over all matched nodes, starting with largest key.
///
/// \param art      ART
/// \param prefix   prefix of nodes to match, may be NULL
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval >0                  \p fold rc
/// \retval <0/E_ART_CALLBACK   \p fold error
/// \retval -1/E_ART_NOT_FOUND  \p prefix not found
int art_fold_pr(art_ct art, str_const_ct prefix, art_fold_cb fold, const void *ctx);

/// Match prefix and fold over all matched nodes, starting with largest key.
///
/// \param art      ART
/// \param prefix   prefix of nodes to match, may be NULL
/// \param fold     callback to invoke on each node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval >0                  \p fold rc
/// \retval <0/E_ART_CALLBACK   \p fold error
/// \retval -1/E_ART_NOT_FOUND  \p prefix not found
/// \retval -1/E_GENERIC_OOM    out of memory
int art_fold_prk(art_ct art, str_const_ct prefix, art_fold_cb fold, const void *ctx);

/// Get full common prefix of all nodes with prefix.
///
/// \param art      ART
/// \param prefix   prefix to complete, may be NULL
///
/// \returns                        full common prefix
/// \retval NULL/E_ART_EMPTY        ART is empty
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found
/// \retval NULL/E_GENERIC_OOM      out of memory
str_ct art_complete(art_const_ct art, str_const_ct prefix);


#endif // ifndef YTIL_CON_ART_H_INCLUDED
