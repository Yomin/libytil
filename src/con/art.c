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

#include <ytil/con/art.h>
#include <ytil/ext/string.h>
#include <ytil/def.h>
#include <ytil/magic.h>
#include <ytil/simd.h>
#include <stdint.h>
#include <stdlib.h>


#define WITH_KEY    true    ///< reconstruct node key
#define WITHOUT_KEY false   ///< do not reconstruct node key
#define FORWARD     false   ///< forward traversal
#define BACKWARD    true    ///< backward traversal
#define NO_PREFIX   NULL    ///< no prefix

#define MAGIC       define_magic("ART") ///< ART magic
#define NODE_MAGIC  define_magic("ARN") ///< ART node magic

/*
 * 2000000 iterations of each method to find key in x bytes
 * 64bit system, intrinsics up to AVX2
 *
 * bytes    linear  binary  memchr  simd
 * 8        0.997   1.010   0.988   0.979
 * 16       1.000   1.016   0.984   0.975
 * 32       1.009   1.028   0.985   0.976
 * 64       1.031   1.050   1.000   0.991 (2 * 32)
 * 128      1.083   1.077   1.017   1.012 (4 * 32)
 *
 * use intrinsics if available
 * binary search produced worst results when it shouldve been superior to linear searches
 * include it nonetheless, other platforms may have optimizations not as good
 * and open for further improvement
 *
 * node sizes (keys+childptrs) for 64bit system
 *
 * bytes    list    index   %       =
 * 4        36      288     12      linear search
 * 8        72      320     22      simd64 / binary search
 * 16       144     384     37      simd128 / binary search
 * 32       288     512     56      simd256 / binary search
 * 48       432     640     67      -
 * 64       576     768     75      index
 * 128      1152    1280    90      index
 * 256      2048    2048    100     id
 *
 * use index search for nodes with size overhead <= 1/4
 * use intrinsics/binary search when suitable intrinsics available
 *
 */


/// ART node type
typedef enum art_node_type
{
    LEAF,       ///< leaf node
    NODE4,      ///< node with 0-4 children
    NODE8,      ///< node with 5-8 children
    NODE16,     ///< node with 9-16 children
    NODE32,     ///< node with 17-32 children
    NODE64,     ///< node with 33-64 children
    NODE128,    ///< node with 65-128 children
    NODE256,    ///< node with 129-256 children
} art_node_id;

/// ART traverse order
typedef enum art_traverse_order
{
    TRAV_LEAF,  ///< leaf node traversal
    TRAV_PRE,   ///< pre-order traversal
    TRAV_IN,    ///< in-order traversal
    TRAV_POST,  ///< post-order traversal
} art_traverse_id;

/// ART leaf node
typedef struct art_leaf
{
    DEBUG_MAGIC
    DEBUG_MEMBER(art_ct art)

    void *data;     ///< node data
} art_leaf_st;

/// ART node4
typedef struct art_node4
{
    unsigned char   key[4];     ///< key list
    art_node_ct     child[4];   ///< child list
} art_node4_st;

/// ART node8
typedef struct art_node8
{
    unsigned char   key[8];     ///< key list
    art_node_ct     child[8];   ///< child list
} art_node8_st;

/// ART node16
typedef struct art_node16
{
    unsigned char   key[16];    ///< key list
    art_node_ct     child[16];  ///< child list
} art_node16_st;

/// ART node32
typedef struct art_node32
{
    unsigned char   key[32];    ///< key list
    art_node_ct     child[32];  ///< child list
} art_node32_st;

/// ART node64
typedef struct art_node64
{
    unsigned char   key[256];   ///< key index
    art_node_ct     child[64];  ///< child list
} art_node64_st;

/// ART node128
typedef struct art_node128
{
    unsigned char   key[256];   ///< key index
    art_node_ct     child[128]; ///< child list
} art_node128_st;

/// ART node256
typedef struct art_node256
{
    art_node_ct child[256];     ///< child index
} art_node256_st;

/// ART node value
typedef union art_value
{
    art_leaf_st     leaf;   ///< leaf node value
    art_node4_st    n4;     ///< node4 value
    art_node8_st    n8;     ///< node8 value
    art_node16_st   n16;    ///< node16 value
    art_node32_st   n32;    ///< node32 value
    art_node64_st   n64;    ///< node64 value
    art_node128_st  n128;   ///< node128 value
    art_node256_st  n256;   ///< node256 value
} art_value_un;

/// ART node
typedef struct art_node
{
    uint8_t         type;       ///< node type
    unsigned char   key;        ///< node key
    uint16_t        size;       ///< number of child nodes

    uint32_t        path_len;   ///< node path length
    unsigned char   *path;      ///< node path

    art_node_ct parent;         ///< parent node

    art_value_un v;             ///< node value
} art_node_st;

/// ART
typedef struct art
{
    DEBUG_MAGIC

    art_node_ct root;       ///< root node
    size_t      size;       ///< number of ART nodes
    bool        ordered;    ///< sort order mode
} art_st;

/// ART error type definition
ERROR_DEFINE_LIST(ART,
    ERROR_INFO(E_ART_CALLBACK,    "Callback error."),
    ERROR_INFO(E_ART_EMPTY,       "ART is empty."),
    ERROR_INFO(E_ART_EXISTS,      "Node already exists."),
    ERROR_INFO(E_ART_INVALID_KEY, "Invalid key."),
    ERROR_INFO(E_ART_NOT_FOUND,   "Node not found.")
);

/// default error type for ART module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_ART


/// Get transient lookup path from key.
///
/// Null terminator is part of string keys.
///
/// \param key  key to get path for
///
/// \returns    transient lookup path
#define art_path_new(key) \
    tstr_new_bs(str_bc(key), str_len(key) + (str_is_binary(key) ? 0 : 1))

/// Get capacity for node type.
///
/// \param type     node type
///
/// \returns        max number of children
#define art_node_capacity(type) (4 << ((type) - NODE4))

/// Get node path.
///
/// \param node     node to get path from
///
/// \returns        node path
static inline unsigned char *art_node_get_path(art_node_const_ct node)
{
    if(node->path_len <= sizeof(unsigned char *))
        return (unsigned char *)&node->path;
    else
        return node->path;
}

/// Drop prefix from node path.
///
/// \param node     node to drop path from
/// \param prefix   number of bytes to drom from path
static void art_node_drop_path(art_node_ct node, size_t prefix)
{
    size_t len = node->path_len - prefix;
    unsigned char *path;

    if(node->path_len <= sizeof(unsigned char *))
    {
        path = (unsigned char *)&node->path;
        memcpy(path, &path[prefix], len);
    }
    else if(len <= sizeof(unsigned char *))
    {
        path = node->path;
        memcpy(&node->path, &node->path[prefix], len);
        free(path);
    }
    else if((path = memdup(&node->path[prefix], len)))
    {
        free(node->path);
        node->path = path;
    }
    else
    {
        memmove(node->path, &node->path[prefix], len);
    }

    node->path_len = len;
}

/// Prepend prefix and key to node path.
///
/// \param node     node to prepend path to
/// \param prefix   prefix to prepend
/// \param len      length of \p prefix
/// \param key      key to prepend
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    ouf of memory
static int art_node_prepend_path(art_node_ct node, const unsigned char *prefix, size_t len, unsigned char key)
{
    unsigned char *path;

    if(node->path_len + len + 1 <= sizeof(unsigned char *))
    {
        path = art_node_get_path(node);
        memmove(&path[len + 1], path, node->path_len);
        memcpy(path, prefix, len);
        path[len] = key;
    }
    else
    {
        if(!(path = malloc(node->path_len + len + 1)))
            return error_wrap_last_errno(malloc), -1;

        memcpy(path, prefix, len);
        path[len] = key;
        memcpy(&path[len + 1], art_node_get_path(node), node->path_len);

        if(node->path_len > sizeof(unsigned char *))
            free(node->path);

        node->path = path;
    }

    node->path_len += len + 1;

    return 0;
}

/// Get length of common prefix of node path and key.
///
/// \param node     node which path to compare
/// \param key      key to compare
///
/// \returns        number of equal bytes in prefix of node path and key
static inline size_t art_node_get_common_prefix(art_node_const_ct node, str_const_ct key)
{
    return memprefix(art_node_get_path(node), str_bc(key),
        MIN(node->path_len, str_len(key)));
}

/// Binary search to find key in key array.
///
/// \param keys     key array to search
/// \param size     number of keys in \p keys
/// \param key      key to find
///
/// \retval >=0     position of \p key in \p keys
/// \retval <0      position -1 where \p key should be in \p keys
static int art_node_find_child(const unsigned char *keys, size_t size, unsigned char key)
{
    size_t left = 0, right = size, pos;

    while(left < right)
    {
        pos = left + (right - left) / 2;

        if(key == keys[pos])
            return pos;

        if(key < keys[pos])
            right = pos;
        else
            left = pos + 1;
    }

    return -1 - left; // negative insert position -1
}

/// Get child node slot from a node4.
///
/// \param node     node to get child from
/// \param key      key of child node
///
/// \returns        child node slot
/// \retval NULL    key not found
static inline art_node_ct *art_node4_get_child(art_node_const_ct node, unsigned char key)
{
    size_t i;

    for(i = 0; i < node->size; i++)
        if(node->v.n4.key[i] == key)
            return (art_node_ct *)&node->v.n4.child[i];

    return NULL;
}

/// Get child node slot from a node8.
///
/// Uses intrinsics if available, else binary search.
///
/// \param node     node to get child from
/// \param key      key of child node
///
/// \returns        child node slot
/// \retval NULL    key not found
static inline art_node_ct *art_node8_get_child(art_node_const_ct node, unsigned char key)
{
#ifdef SIMD64
    int index = simd64_index8(node->v.n8.key, node->size, key);
#else
    int index = art_node_find_child(node->v.n8.key, node->size, key);
#endif
    return index < 0 ? NULL : (art_node_ct *)&node->v.n8.child[index];
}

/// Get child node slot from a node16.
///
/// Uses intrinsics if available, else binary search.
///
/// \param node     node to get child from
/// \param key      key of child node
///
/// \returns        child node slot
/// \retval NULL    key not found
static inline art_node_ct *art_node16_get_child(art_node_const_ct node, unsigned char key)
{
#ifdef SIMD128
    int index = simd128_index8(node->v.n16.key, node->size, key);
#else
    int index = art_node_find_child(node->v.n16.key, node->size, key);
#endif
    return index < 0 ? NULL : (art_node_ct *)&node->v.n16.child[index];
}

/// Get child node slot from a node32.
///
/// Uses intrinsics if available, else binary search.
///
/// \param node     node to get child from
/// \param key      key of child node
///
/// \returns        child node slot
/// \retval NULL    key not found
static inline art_node_ct *art_node32_get_child(art_node_const_ct node, unsigned char key)
{
#ifdef SIMD256
    int index = simd256_index8(node->v.n32.key, node->size, key);
#else
    int index = art_node_find_child(node->v.n32.key, node->size, key);
#endif
    return index < 0 ? NULL : (art_node_ct *)&node->v.n32.child[index];
}

/// Get child node slot from node.
///
/// \param node     node to get child from
/// \param key      key of child node
///
/// \returns        child node slot
/// \retval NULL    key not found
static art_node_ct *art_node_get_child(art_node_const_ct node, unsigned char key)
{
    uint8_t index;

    switch(node->type)
    {
    case NODE4:
        return art_node4_get_child(node, key);

    case NODE8:
        return art_node8_get_child(node, key);

    case NODE16:
        return art_node16_get_child(node, key);

    case NODE32:
        return art_node32_get_child(node, key);

    case NODE64:
        index = node->v.n64.key[key];

        return index == 0xff ? NULL : (art_node_ct *)&node->v.n64.child[index];

    case NODE128:
        index = node->v.n128.key[key];

        return index == 0xff ? NULL : (art_node_ct *)&node->v.n128.child[index];

    case NODE256:
        return node->v.n256.child[key] ? (art_node_ct *)&node->v.n256.child[key] : NULL;

    default:
        abort();
    }
}

/// Find node which full path best matches key.
///
/// \param      art     ART
/// \param      node    node to start with
/// \param      key     key to match, parent prefix is cut off
/// \param[out] prefix  on match contains matched prefix of node path, may be NULL
/// \param[out] slot    on match contains node's slot pointer in parent node, may be NULL
///
/// \returns        best matching node
/// \retval NULL    \p node is NULL
static art_node_ct art_lookup(art_const_ct art, art_node_const_ct node, str_ct key, size_t *prefix, art_node_ct **slot)
{
    art_node_ct *child, *node_slot;
    size_t prefixlen;

    return_value_if_fail(node, NULL);

    if(slot)
    {
        if(node == art->root)
            *slot = (art_node_ct *)&art->root;
        else
            *slot = art_node_get_child(node->parent, node->key);
    }

    prefix  = prefix ? prefix : &prefixlen;
    slot    = slot ? slot : &node_slot;

    while(1)
    {
        *prefix = art_node_get_common_prefix(node, key);
        str_cut_head(key, *prefix);

        if(*prefix < node->path_len)
            return (art_node_ct)node;

        if(node->type == LEAF)
            return (art_node_ct)node;

        if(str_is_empty(key))
            return (art_node_ct)node;

        if(!(child = art_node_get_child(node, str_first_u(key))))
            return (art_node_ct)node;

        str_cut_head(key, 1);
        node    = *child;
        *slot   = child;
    }
}

/// ART traverse callback.
///
/// \param art      ART
/// \param node     node
/// \param path     reconstructed node key, may be NULL
/// \param ctx      callback context
///
/// \retval 0           success
/// \retval <0/error    stop traversal with error
/// \retval >0          stop traversal
typedef int (*art_traverse_cb)(art_ct art, art_node_ct node, str_ct path, void *ctx);

/// ART traverse state
typedef struct art_traverse_state
{
    art_traverse_id order;      ///< traverse order
    str_ct          path;       ///< traverse path
    bool            reverse;    ///< reverse traverse direction
    art_traverse_cb cb;         ///< traverse callback
    void            *ctx;       ///< traverse callback context
} art_traverse_st;

static int art_traverse_node(art_ct art, size_t pos, art_node_ct node, art_traverse_st *state);

/// Traverse children of list nodes.
///
/// \param art      ART
/// \param children children to traverse
/// \param size     number of children
/// \param state    traverse state
///
/// \returns        traverse rc
static int art_traverse_list(art_ct art, art_node_ct *children, size_t size, art_traverse_st *state)
{
    art_node_ct child;
    size_t i;
    int rc;

    for(i = 0; i < size; i++)
    {
        child = children[state->reverse ? size - i - 1 : i];

        if((rc = art_traverse_node(art, i, child, state)))
            return rc;
    }

    return 0;
}

/// Traverse children of index nodes.
///
/// \param art      ART
/// \param keys     keys to traverse
/// \param children children to traverse
/// \param size     number of children
/// \param state    traverse state
///
/// \returns        traverse rc
static int art_traverse_index(art_ct art, unsigned char *keys, art_node_ct *children, size_t size, art_traverse_st *state)
{
    unsigned char key;
    size_t i, pos;
    int rc;

    for(i = 0, pos = 0; pos < size; i++)
        if((key = keys[state->reverse ? 255 - i : i]) != 0xff)
        {
            if((rc = art_traverse_node(art, pos, children[key], state)))
                return rc;

            pos++;
        }

    return 0;
}

/// Traverse children of n256 nodes.
///
/// \param art      ART
/// \param node     node which children are traversed
/// \param state    traverse state
///
/// \returns        traverse rc
static inline int art_traverse_node256(art_ct art, art_node_ct node, art_traverse_st *state)
{
    art_node_ct child;
    size_t i, pos;
    int rc;

    for(i = 0, pos = 0; pos < node->size; i++)
        if((child = node->v.n256.child[state->reverse ? 255 - i : i]))
        {
            if((rc = art_traverse_node(art, pos, child, state)))
                return rc;

            pos++;
        }

    return 0;
}

/// Traverse children of node.
///
/// \param art      ART
/// \param pos      position of node in traverse direction
/// \param node     node which children to traverse
/// \param state    traverse state
///
/// \returns                    traverse rc
/// \retval -1/E_GENERIC_OOM    out of memory
static int art_traverse_node(art_ct art, size_t pos, art_node_ct node, art_traverse_st *state)
{
    size_t path_len = node->path_len;
    int rc          = 0;

    if(  state->order == TRAV_IN && node->parent && pos > 0
      && (rc = error_pass_int(state->cb(art, node->parent, state->path, state->ctx))))
        return rc;

    if(state->path)
    {
        if(  (node->parent && !str_append_set(state->path, 1, node->key))
          || !str_append_b(state->path, art_node_get_path(node), path_len))
            return error_wrap(), -1;

        if(node->parent)
            path_len++; // remember to drop node->key

    }

    if(node->type == LEAF)
    {
        rc = error_pass_int(state->cb(art, node, state->path, state->ctx));
    }
    else
    {
        if(state->order == TRAV_PRE)
            rc = error_pass_int(state->cb(art, node, state->path, state->ctx));

        if(!rc)
        {
            switch(node->type)
            {
            case NODE4:
                rc = art_traverse_list(art, node->v.n4.child, node->size, state);
                break;

            case NODE8:
                rc = art_traverse_list(art, node->v.n8.child, node->size, state);
                break;

            case NODE16:
                rc = art_traverse_list(art, node->v.n16.child, node->size, state);
                break;

            case NODE32:
                rc = art_traverse_list(art, node->v.n32.child, node->size, state);
                break;

            case NODE64:
                rc = art_traverse_index(art,
                    node->v.n64.key, node->v.n64.child, node->size, state);
                break;

            case NODE128:
                rc = art_traverse_index(art,
                    node->v.n128.key, node->v.n128.child, node->size, state);
                break;

            case NODE256:
                rc = art_traverse_node256(art, node, state);
                break;

            default:
                abort();
            }
        }

        if(!rc && state->order == TRAV_POST)
            rc = error_pass_int(state->cb(art, node, state->path, state->ctx));
    }

    if(state->path)
        str_cut_tail(state->path, path_len);

    return rc;
}

/// Traverse ART.
///
/// \param art      ART
/// \param node     node to start with
/// \param prefix   prefix to lookup first, may be NULL
/// \param order    traverse order
/// \param key      if true reconstruct node key
/// \param reverse  if true begin with largest key else smallest key
/// \param traverse callback to invoke on each node
/// \param ctx      \p traverse context
///
/// \returns                    \p traverse rc
/// \retval -1/E_ART_NOT_FOUND  \p node is NULL or no node with \p prefix found
/// \retval -1/E_GENERIC_OOM    out of memory
static int art_traverse(art_ct art, art_node_ct node, str_const_ct prefix, art_traverse_id order, bool key, bool reverse, art_traverse_cb traverse, void *ctx)
{
    art_traverse_st state = { .order = order, .reverse = reverse, .cb = traverse, .ctx = ctx };
    str_ct lookup;
    size_t node_prefix;
    int rc;

    return_error_if_fail(node, E_ART_NOT_FOUND, -1);

    if(prefix)
    {
        lookup  = art_path_new(prefix);
        node    = art_lookup(art, node, lookup, &node_prefix, NULL);
        return_error_if_fail(str_is_empty(lookup), E_ART_NOT_FOUND, -1);

        if(node->parent)
            node_prefix++; // dont dup node->key

        if(key && !(state.path = str_dup_n(prefix, str_len(prefix) - node_prefix)))
            return error_wrap(), -1;
    }
    else
    {
        if(key && !(state.path = str_prepare_bc(0, 10)))
            return error_wrap(), -1;
    }

    if(key)
        str_mark_volatile(state.path);

    rc = error_pass_int(art_traverse_node(art, 0, node, &state));

    if(key)
        str_unref(state.path);

    return rc;
}

art_ct art_new(art_mode_id mode)
{
    art_ct art;

    if(!(art = calloc(1, sizeof(art_st))))
        return error_wrap_last_errno(calloc), NULL;

    init_magic(art);

    art->ordered = mode == ART_MODE_ORDERED;

    return art;
}

static int art_node_remove(art_ct art, art_node_ct node, str_const_ct prefix, art_dtor_cb dtor, const void *ctx);

void art_free(art_ct art)
{
    assert_magic(art);

    art_node_remove(art, art->root, NO_PREFIX, NULL, NULL);
    free(art);
}

void art_free_f(art_ct art, art_dtor_cb dtor, const void *ctx)
{
    assert_magic(art);

    art_node_remove(art, art->root, NO_PREFIX, dtor, ctx);
    free(art);
}

art_ct art_free_if_empty(art_ct art)
{
    return art_free_if_empty_f(art, NULL, NULL);
}

art_ct art_free_if_empty_f(art_ct art, art_dtor_cb dtor, const void *ctx)
{
    assert_magic(art);

    if(art->size)
        return art;

    art_free_f(art, dtor, ctx);

    return NULL;
}

void art_clear(art_ct art)
{
    assert_magic(art);

    art_node_remove(art, art->root, NO_PREFIX, NULL, NULL);
}

void art_clear_f(art_ct art, art_dtor_cb dtor, const void *ctx)
{
    assert_magic(art);

    art_node_remove(art, art->root, NO_PREFIX, dtor, ctx);
}

bool art_is_empty(art_const_ct art)
{
    assert_magic(art);

    return !art->size;
}

size_t art_size(art_const_ct art)
{
    assert_magic(art);

    return art->size;
}

/// Get ART node size.
///
/// \param type     node type
///
/// \returns        ART node size in bytes.
static size_t art_node_size(art_node_id type)
{
    size_t size = sizeof(art_node_st) - sizeof(art_value_un);

    switch(type)
    {
    case LEAF:
        return size + sizeof(art_leaf_st);

    case NODE4:
        return size + sizeof(art_node4_st);

    case NODE8:
        return size + sizeof(art_node8_st);

    case NODE16:
        return size + sizeof(art_node16_st);

    case NODE32:
        return size + sizeof(art_node32_st);

    case NODE64:
        return size + sizeof(art_node64_st);

    case NODE128:
        return size + sizeof(art_node128_st);

    case NODE256:
        return size + sizeof(art_node256_st);

    default:
        abort();
    }
}

/// ART memsize state
typedef struct art_memsize_state
{
    art_size_cb nsize;  ///< node data size callback
    void        *ctx;   ///< node data size callback context
    size_t      size;   ///< size accumulator
} art_memsize_st;

/// ART traverse callback for accumulating allocated ART size.
///
/// \implements art_traverse_cb
///
/// \retval 0   always success
static int art_traverse_memsize(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_memsize_st *state = ctx;

    state->size += art_node_size(node->type);

    if(node->path_len > sizeof(unsigned char *))
        state->size += node->path_len;

    if(node->type == LEAF && state->nsize)
        state->size += state->nsize(art, node->v.leaf.data, state->ctx);

    return 0;
}

size_t art_memsize(art_const_ct art)
{
    return art_memsize_f(art, NULL, NULL);
}

size_t art_memsize_f(art_const_ct art, art_size_cb size, const void *ctx)
{
    art_memsize_st state = { .nsize = size, .ctx = (void *)ctx, .size = sizeof(art_st) };

    assert_magic(art);

    art_traverse((art_ct)art, art->root, NO_PREFIX, TRAV_PRE, WITHOUT_KEY, FORWARD, art_traverse_memsize, &state);

    return state.size;
}

str_ct art_node_key(art_node_const_ct node)
{
    str_ct key;

    assert_magic_n(&node->v.leaf, NODE_MAGIC);

    if(!(key = str_dup_b(art_node_get_path(node), node->path_len)))
        return error_wrap(), NULL;

    for(; node->parent; node = node->parent)
        if(  !str_prepend_b(key, &node->key, 1)
          || !str_prepend_b(key, art_node_get_path(node->parent), node->parent->path_len))
            return error_wrap(), str_unref(key), NULL;

    return key;
}

void *art_node_data(art_node_const_ct node)
{
    assert_magic_n(&node->v.leaf, NODE_MAGIC);

    return node->v.leaf.data;
}

void art_node_set(art_node_ct node, const void *data)
{
    assert_magic_n(&node->v.leaf, NODE_MAGIC);

    node->v.leaf.data = (void *)data;
}

art_node_ct art_get(art_const_ct art, str_const_ct key)
{
    art_node_ct node;
    str_ct path;

    assert_magic(art);

    path = art_path_new(key);

    if(  !(node = art_lookup(art, art->root, path, NULL, NULL))
      || node->type != LEAF
      || !str_is_empty(path))
        return error_set(E_ART_NOT_FOUND), NULL;

    return node;
}

void *art_get_data(art_const_ct art, str_const_ct key)
{
    art_node_ct node;

    if(!(node = art_get(art, key)))
        return error_pass(), NULL;

    return node->v.leaf.data;
}

/// Create new ART node.
///
/// \param type     node type
/// \param key      node key
/// \param parent   parent node
/// \param path     node path to set, may be NULL
///
/// \returns                    new ART node
/// \retval NULL/E_GENERIC_OOM  out of memory
static art_node_ct art_node_new(art_node_id type, unsigned char key, art_node_ct parent, str_const_ct path)
{
    art_node_ct node;

    if(!(node = calloc(1, art_node_size(type))))
        return error_wrap_last_errno(calloc), NULL;

    if(path)
    {
        if(str_len(path) <= sizeof(unsigned char *))
            memcpy(&node->path, str_bc(path), str_len(path));
        else if(!(node->path = memdup(str_bc(path), str_len(path))))
            return error_wrap_last_errno(memdup), NULL;
    }

    switch(type)
    {
    case NODE64:
        memset(node->v.n64.key, 0xff, sizeof(node->v.n64.key));
        break;

    case NODE128:
        memset(node->v.n128.key, 0xff, sizeof(node->v.n128.key));
        break;

    default:
        break;
    }

    node->type      = type;
    node->path_len  = path ? str_len(path) : 0;
    node->key       = key;
    node->parent    = parent;

    return node;
}

/// Create new ART leaf node.
///
/// \param art      ART node belongs to
/// \param path     node path to set, may be NULL
/// \param data     node data to set
///
/// \returns                    new ART leaf node
/// \retval NULL/E_GENERIC_OOM  out of memory
static art_node_ct art_node_new_leaf(art_ct art, str_const_ct path, const void *data)
{
    art_node_ct node;

    if(!(node = art_node_new(LEAF, '\0', NULL, path)))
        return error_pass(), NULL;

    init_magic_n(&node->v.leaf, NODE_MAGIC);
    DEBUG(node->v.leaf.art  = art);
    node->v.leaf.data       = (void *)data;

    return node;
}

/// Copy list node to another list node.
///
/// \param node2        destination node
/// \param keys2        destination keys
/// \param children2    destination children
/// \param node1        source node
/// \param keys1        source keys
/// \param children1    source children
static inline void art_node_copy_list(art_node_ct node2, unsigned char *keys2, art_node_ct *children2, art_node_ct node1, unsigned char *keys1, art_node_ct *children1)
{
    size_t n;

    memcpy(keys2, keys1, node1->size);
    memcpy(children2, children1, node1->size * sizeof(art_node_ct));

    for(n = 0; n < node1->size; n++)
        children1[n]->parent = node2;
}

/// Copy node32 to node64.
///
/// \param n64      destination node
/// \param n32      source node
static inline void art_node32_to_node64(art_node_ct n64, art_node_ct n32)
{
    size_t n;

    for(n = 0; n < n32->size; n++)
    {
        n64->v.n64.key[n32->v.n32.key[n]]   = n;
        n64->v.n64.child[n]                 = n32->v.n32.child[n];
        n64->v.n64.child[n]->parent         = n64;
    }
}

/// Copy node64 to node128.
///
/// \param n128     destination node
/// \param n64      source node
static inline void art_node64_to_node128(art_node_ct n128, art_node_ct n64)
{
    size_t n;

    memcpy(n128->v.n128.key, n64->v.n64.key, 256);
    memcpy(n128->v.n128.child, n64->v.n64.child, n64->size * sizeof(art_node_ct));

    for(n = 0; n < n64->size; n++)
        n64->v.n64.child[n]->parent = n128;
}

/// Copy node128 to node256.
///
/// \param n256     destination node
/// \param n128     source node
static inline void art_node128_to_node256(art_node_ct n256, art_node_ct n128)
{
    size_t n;

    for(n = 0; n < 256; n++)
        if(n128->v.n128.key[n] != 0xff)
        {
            n256->v.n256.child[n]           = n128->v.n128.child[n128->v.n128.key[n]];
            n256->v.n256.child[n]->parent   = n256;
        }
}

/// Grow ART node to next bigger size.
///
/// \param slot     slot of node to grow
///
/// \returns                    new node
/// \retval NULL/E_GENERIC_OOM  out of memory
static art_node_ct art_node_grow(art_node_ct *slot)
{
    art_node_ct node1 = *slot, node2;

    if(!(node2 = art_node_new(node1->type + 1, node1->key, node1->parent, NULL)))
        return error_pass(), NULL;

    switch(node1->type)
    {
    case NODE4:
        art_node_copy_list(node2, node2->v.n8.key, node2->v.n8.child,
            node1, node1->v.n4.key, node1->v.n4.child);
        break;

    case NODE8:
        art_node_copy_list(node2, node2->v.n16.key, node2->v.n16.child,
            node1, node1->v.n8.key, node1->v.n8.child);
        break;

    case NODE16:
        art_node_copy_list(node2, node2->v.n32.key, node2->v.n32.child,
            node1, node1->v.n16.key, node1->v.n16.child);
        break;

    case NODE32:
        art_node32_to_node64(node2, node1);
        break;

    case NODE64:
        art_node64_to_node128(node2, node1);
        break;

    case NODE128:
        art_node128_to_node256(node2, node1);
        break;

    default:
        abort();
    }

    node2->size     = node1->size;
    node2->path     = node1->path;
    node2->path_len = node1->path_len;

    *slot = node2;
    free(node1);

    return node2;
}

/// Insert child node into list parent.
///
/// \param parent   parent to insert child into
/// \param keys     parent keys
/// \param children parent children
/// \param key      key to insert into
/// \param child    child node to insert
/// \param index    position to insert at
static inline void art_node_list_insert(art_node_ct parent, unsigned char *keys, art_node_ct *children, unsigned char key, art_node_ct child, int index)
{
    memmove(&keys[index + 1], &keys[index], parent->size - index);
    memmove(&children[index + 1], &children[index], (parent->size - index) * sizeof(art_node_ct));

    keys[index]     = key;
    children[index] = child;
}

#if defined(SIMD64) || defined(SIMD128) || defined(SIMD256)
/// Append child node to list parent.
///
/// \param parent   parent to append child to
/// \param keys     parent keys
/// \param children parent children
/// \param key      key to append with
/// \param child    child node to append
static inline void art_node_list_append(art_node_ct parent, unsigned char *keys, art_node_ct *children, unsigned char key, art_node_ct child)
{
    keys[parent->size]      = key;
    children[parent->size]  = child;
}
#endif

/// Insert child node into index parent.
///
/// \param parent   parent to insert child into
/// \param keys     parent keys
/// \param children parent children
/// \param key      key to insert into
/// \param child    child node to insert
static inline void art_node_insert_index(art_node_ct parent, unsigned char *keys, art_node_ct *children, unsigned char key, art_node_ct child)
{
    keys[key]               = parent->size;
    children[parent->size]  = child;
}

/// Insert child node into node4.
///
/// \param parent   parent to insert child into
/// \param key      key to insert into
/// \param child    child node to insert
static inline void art_node4_insert(art_node_ct parent, unsigned char key, art_node_ct child)
{
    size_t k;

    for(k = 0; k < parent->size; k++)
        if(key < parent->v.n4.key[k])
            break;

    art_node_list_insert(parent, parent->v.n4.key, parent->v.n4.child, key, child, k);
}

/// Insert child node into node4.
///
/// \param parent   parent to insert child into
/// \param key      key to insert into
/// \param child    child node to insert
/// \param ordered  if true insert keeping ascending order
static inline void art_node8_insert(art_node_ct parent, unsigned char key, art_node_ct child, bool ordered)
{
#ifdef SIMD64

    if(!ordered)
        art_node_list_append(parent, parent->v.n8.key, parent->v.n8.child, key, child);
    else
#endif
    art_node_list_insert(parent, parent->v.n8.key, parent->v.n8.child, key, child,
        -art_node_find_child(parent->v.n8.key, parent->size, key) - 1);
}

/// Insert child node into node16.
///
/// \param parent   parent to insert child into
/// \param key      key to insert into
/// \param child    child node to insert
/// \param ordered  if true insert keeping ascending order
static inline void art_node16_insert(art_node_ct parent, unsigned char key, art_node_ct child, bool ordered)
{
#ifdef SIMD128

    if(!ordered)
        art_node_list_append(parent, parent->v.n16.key, parent->v.n16.child, key, child);
    else
#endif
    art_node_list_insert(parent, parent->v.n16.key, parent->v.n16.child, key, child,
        -art_node_find_child(parent->v.n16.key, parent->size, key) - 1);
}

/// Insert child node into node32.
///
/// \param parent   parent to insert child into
/// \param key      key to insert into
/// \param child    child node to insert
/// \param ordered  if true insert keeping ascending order
static inline void art_node32_insert(art_node_ct parent, unsigned char key, art_node_ct child, bool ordered)
{
#ifdef SIMD256

    if(!ordered)
        art_node_list_append(parent, parent->v.n32.key, parent->v.n32.child, key, child);
    else
#endif
    art_node_list_insert(parent, parent->v.n32.key, parent->v.n32.child, key, child,
        -art_node_find_child(parent->v.n32.key, parent->size, key) - 1);
}

/// Insert child node into parent node.
///
/// \param parent   parent node to insert child into
/// \param key      key to insert info
/// \param child    child node to insert
/// \param ordered  if true insert keeping ascending order
///
/// \returns        \p child
static art_node_ct art_node_insert(art_node_ct parent, unsigned char key, art_node_ct child, bool ordered)
{
    switch(parent->type)
    {
    case NODE4:
        art_node4_insert(parent, key, child);
        break;

    case NODE8:
        art_node8_insert(parent, key, child, ordered);
        break;

    case NODE16:
        art_node16_insert(parent, key, child, ordered);
        break;

    case NODE32:
        art_node32_insert(parent, key, child, ordered);
        break;

    case NODE64:
        art_node_insert_index(parent, parent->v.n64.key, parent->v.n64.child, key, child);
        break;

    case NODE128:
        art_node_insert_index(parent, parent->v.n128.key, parent->v.n128.child, key, child);
        break;

    case NODE256:
        parent->v.n256.child[key] = child;
        break;

    default:
        abort();
    }

    parent->size++;
    child->key      = key;
    child->parent   = parent;

    return child;
}

/// Split node path and insert new node.
///
/// \param art      ART
/// \param slot     slot of node to split
/// \param prefix   number of bytes to cut from child and assign to new parent
///
/// \returns                    new parent node
/// \retval NULL/E_GENERIC_OOM  out of memory
static art_node_ct art_node_split(art_ct art, art_node_ct *slot, size_t prefix)
{
    art_node_ct child = *slot, inode;
    unsigned char *path = art_node_get_path(child);

    assert(prefix + 1 <= child->path_len);

    if(!(inode = art_node_new(NODE4, child->key, child->parent, tstr_new_bs(path, prefix))))
        return error_pass(), NULL;

    *slot = inode;

    art_node_insert(inode, path[prefix], child, art->ordered);
    art_node_drop_path(child, prefix + 1);

    return inode;
}

/// Free ART node
///
/// \param node     node to free
static void art_node_free(art_node_ct node)
{
    if(node->path_len > sizeof(unsigned char *))
        free(node->path);

    free(node);
}

art_node_ct art_insert(art_ct art, str_const_ct key, const void *data)
{
    art_node_ct leaf, node, *node_slot;
    unsigned char leaf_key;
    str_ct path;
    size_t prefix_len;

    assert_magic(art);

    path = art_path_new(key);
    return_error_if_pass(str_is_empty(path), E_ART_INVALID_KEY, NULL);

    if(!art->root)
    {
        if(!(leaf = art_node_new_leaf(art, path, data)))
            return error_pass(), NULL;

        art->root = leaf;
        art->size++;

        return leaf;
    }

    node = art_lookup(art, art->root, path, &prefix_len, &node_slot);

    if(  str_is_empty(path)
      || (prefix_len == node->path_len && node->type == LEAF))
        return error_set(E_ART_EXISTS), NULL;

    leaf_key = str_first_u(path);
    str_cut_head(path, 1);

    if(!(leaf = art_node_new_leaf(art, path, data)))
        return error_pass(), NULL;

    if(prefix_len < node->path_len)
    {
        if(!(node = art_node_split(art, node_slot, prefix_len)))
            return error_pass(), art_node_free(leaf), NULL;
    }
    else
    {
        if(  node->size == art_node_capacity(node->type)
          && !(node = art_node_grow(node_slot)))
            return error_pass(), art_node_free(leaf), NULL;
    }

    art->size++;

    return art_node_insert(node, leaf_key, leaf, art->ordered);
}

/// Remove child node from list node while keeping order.
///
/// \param parent   parent to remove child from
/// \param keys     parent keys
/// \param children parent children
/// \param index    position of key to remove
static inline void art_node_remove_child_sorted(art_node_ct parent, unsigned char *keys, art_node_ct *children, size_t index)
{
    if(index + 1 < parent->size)
    {
        memmove(&keys[index], &keys[index + 1], parent->size - index - 1);
        memmove(&children[index], &children[index + 1], (parent->size - index - 1) * sizeof(art_node_ct));
    }
}

#if defined(SIMD64) || defined(SIMD128) || defined(SIMD256)
/// Remove child node from list node without keeping order.
///
/// \param parent   parent to remove child from
/// \param keys     parent keys
/// \param children parent children
/// \param index    position of key to remove
static inline void art_node_remove_child_unsorted(art_node_ct parent, unsigned char *keys, art_node_ct *children, size_t index)
{
    if(index + 1 < parent->size)
    {
        keys[index]     = keys[parent->size - 1];
        children[index] = children[parent->size - 1];
    }
}
#endif

/// Remove child node from index node.
///
/// \param parent   parent to remove child from
/// \param keys     parent keys
/// \param children parent children
/// \param key      key to remove
static inline void art_node_remove_child_index(art_node_ct parent, unsigned char *keys, art_node_ct *children, unsigned char key)
{
    art_node_ct last;
    size_t i;

    if(parent->size > 1)
    {
        last = children[parent->size - 1];

        for(i = 0; i < 256; i++)
            if(keys[i] != 0xff && children[keys[i]] == last)
            {
                keys[i]             = keys[key];
                children[keys[i]]   = last;
                break;
            }
    }

    keys[key] = 0xff;
}

/// Remove child node from node4.
///
/// \param parent   parent to remove child from
/// \param key      key to remove
static inline void art_node4_remove_child(art_node_ct parent, unsigned char key)
{
    size_t i;

    for(i = 0; i < parent->size; i++)
        if(parent->v.n4.key[i] == key)
        {
            art_node_remove_child_sorted(parent, parent->v.n4.key, parent->v.n4.child, i);
            return;
        }
}

/// Remove child node from node8.
///
/// \param parent   parent to remove child from
/// \param key      key to remove
/// \param ordered  if true remove keeping ascending order
static inline void art_node8_remove_child(art_node_ct parent, unsigned char key, bool ordered)
{
#ifdef SIMD64

    if(ordered)
        art_node_remove_child_sorted(parent, parent->v.n8.key, parent->v.n8.child,
            simd64_index8(parent->v.n8.key, parent->size, key));
    else
        art_node_remove_child_unsorted(parent, parent->v.n8.key, parent->v.n8.child,
            simd64_index8(parent->v.n8.key, parent->size, key));

#else
    art_node_remove_child_sorted(parent, parent->v.n8.key, parent->v.n8.child,
        art_node_find_child(parent->v.n8.key, parent->size, key));
#endif
}

/// Remove child node from node16.
///
/// \param parent   parent to remove child from
/// \param key      key to remove
/// \param ordered  if true remove keeping ascending order
static inline void art_node16_remove_child(art_node_ct parent, unsigned char key, bool ordered)
{
#ifdef SIMD128

    if(ordered)
        art_node_remove_child_sorted(parent, parent->v.n16.key, parent->v.n16.child,
            simd128_index8(parent->v.n16.key, parent->size, key));
    else
        art_node_remove_child_unsorted(parent, parent->v.n16.key, parent->v.n16.child,
            simd128_index8(parent->v.n16.key, parent->size, key));

#else
    art_node_remove_child_sorted(parent, parent->v.n16.key, parent->v.n16.child,
        art_node_find_child(parent->v.n16.key, parent->size, key));
#endif
}

/// Remove child node from node32.
///
/// \param parent   parent to remove child from
/// \param key      key to remove
/// \param ordered  if true remove keeping ascending order
static inline void art_node32_remove_child(art_node_ct parent, unsigned char key, bool ordered)
{
#ifdef SIMD256

    if(ordered)
        art_node_remove_child_sorted(parent, parent->v.n32.key, parent->v.n32.child,
            simd256_index8(parent->v.n32.key, parent->size, key));
    else
        art_node_remove_child_unsorted(parent, parent->v.n32.key, parent->v.n32.child,
            simd256_index8(parent->v.n32.key, parent->size, key));

#else
    art_node_remove_child_sorted(parent, parent->v.n32.key, parent->v.n32.child,
        art_node_find_child(parent->v.n32.key, parent->size, key));
#endif
}

/// Remove child node from parent node.
///
/// \param parent   parent to remove child from
/// \param key      key to remove
/// \param ordered  if true remove keeping ascending order
static void art_node_remove_child(art_node_ct parent, unsigned char key, bool ordered)
{
    switch(parent->type)
    {
    case NODE4:
        art_node4_remove_child(parent, key);
        break;

    case NODE8:
        art_node8_remove_child(parent, key, ordered);
        break;

    case NODE16:
        art_node16_remove_child(parent, key, ordered);
        break;

    case NODE32:
        art_node32_remove_child(parent, key, ordered);
        break;

    case NODE64:
        art_node_remove_child_index(parent, parent->v.n64.key, parent->v.n64.child, key);
        break;

    case NODE128:
        art_node_remove_child_index(parent, parent->v.n128.key, parent->v.n128.child, key);
        break;

    case NODE256:
        parent->v.n256.child[key] = NULL;
        break;

    default:
        abort();
    }

    parent->size--;
}

/// Copy node64 to node32.
///
/// \param n32      destination node
/// \param n64      source node
static inline void art_node64_to_node32(art_node_ct n32, art_node_ct n64)
{
    size_t n, s;

    for(n = 0, s = 0; n < 256 && s < n64->size; n++)
        if(n64->v.n64.key[n] != 0xff)
        {
            n32->v.n32.key[s]           = n;
            n32->v.n32.child[s]         = n64->v.n64.child[n64->v.n64.key[n]];
            n32->v.n32.child[s]->parent = n32;
            s++;
        }
}

/// Copy node128 to node64.
///
/// \param n64      destination node
/// \param n128     source node
static inline void art_node128_to_node64(art_node_ct n64, art_node_ct n128)
{
    size_t n;

    memcpy(n64->v.n64.key, n128->v.n128.key, 256);
    memcpy(n64->v.n64.child, n128->v.n128.child, n128->size * sizeof(art_node_ct));

    for(n = 0; n < n128->size; n++)
        n128->v.n128.child[n]->parent = n64;
}

/// Copy node256 to node128.
///
/// \param n128     destination node
/// \param n256     source node
static inline void art_node256_to_node128(art_node_ct n128, art_node_ct n256)
{
    size_t n, s;

    for(n = 0, s = 0; n < 256 && s < n256->size; n++)
        if(n256->v.n256.child[n])
        {
            n128->v.n128.key[n]             = s;
            n128->v.n128.child[s]           = n256->v.n256.child[n];
            n128->v.n128.child[s]->parent   = n128;
            s++;
        }
}

/// Shrink ART node to next smaller size.
///
/// \param art      ART
/// \param node1    node to shrink
///
/// \returns                    new node
/// \retval NULL/E_GENERIC_OOM  out of memory
static art_node_ct art_node_shrink(art_ct art, art_node_ct node1)
{
    art_node_ct node2;

    if(!(node2 = art_node_new(node1->type - 1, node1->key, node1->parent, NULL)))
        return error_pass(), NULL;

    switch(node1->type)
    {
    case NODE8:
        art_node_copy_list(node2, node2->v.n4.key, node2->v.n4.child,
            node1, node1->v.n8.key, node1->v.n8.child);
        break;

    case NODE16:
        art_node_copy_list(node2, node2->v.n8.key, node2->v.n8.child,
            node1, node1->v.n16.key, node1->v.n16.child);
        break;

    case NODE32:
        art_node_copy_list(node2, node2->v.n16.key, node2->v.n16.child,
            node1, node1->v.n32.key, node1->v.n32.child);
        break;

    case NODE64:
        art_node64_to_node32(node2, node1);
        break;

    case NODE128:
        art_node128_to_node64(node2, node1);
        break;

    case NODE256:
        art_node256_to_node128(node2, node1);
        break;

    default:
        abort();
    }

    node2->size     = node1->size;
    node2->path     = node1->path;
    node2->path_len = node1->path_len;

    if(node1 == art->root)
        art->root = node2;
    else
        *art_node_get_child(node1->parent, node1->key) = node2;

    free(node1);

    return node2;
}

/// ART traverse callback for getting first child node.
///
/// \implements art_traverse_cb
///
/// \retval 1   always stop on first node
static int art_traverse_first(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_node_ct *child = ctx;

    *child = node;

    return 1;
}

/// Get first child node of parent node.
///
/// \param parent   parent to get child from
///
/// \returns        first child node
static art_node_ct art_node_get_first_child(art_node_ct parent)
{
    art_node_ct child = NULL;

    art_traverse(NULL, parent, NO_PREFIX, TRAV_POST, WITHOUT_KEY, FORWARD, art_traverse_first, &child);

    return child;
}

/// Merge node with its only child.
///
/// \param art      ART
/// \param node     node to merge
///
/// \returns                    child node
/// \retval NULL/E_GENERIC_OOM  out of memory
static art_node_ct art_node_merge(art_ct art, art_node_ct node)
{
    art_node_ct child;

    // node should be NODE4, but if shrink failed we may have any other type
    child = art_node_get_first_child(node);

    if(art_node_prepend_path(child, art_node_get_path(node), node->path_len, child->key))
        return error_pass(), NULL;

    child->parent   = node->parent;
    child->key      = node->key;

    if(node == art->root)
        art->root = child;
    else
        *art_node_get_child(node->parent, node->key) = child;

    art_node_free(node);

    return child;
}

/// Destroy and free ART node.
///
/// \param art      ART
/// \param node     node to free
/// \param dtor     callback to destroy leaf node data, may be NULL
/// \param ctx      \p dtor context
static void art_node_free_f(art_ct art, art_node_ct node, art_dtor_cb dtor, const void *ctx)
{
    if(node->type == LEAF)
    {
        if(dtor)
            dtor(art, node->v.leaf.data, (void *)ctx);

        art->size--;
    }

    art_node_free(node);
}

/// ART remove state
typedef struct art_remove_state
{
    art_dtor_cb dtor;   ///< node dtor
    void        *ctx;   ///< node dtor context
    art_node_ct last;   ///< last node marked for removal
} art_remove_st;

/// ART traverse callback for removing nodes.
///
/// \implements art_traverse_cb
///
/// \retval 0   always success
static int art_traverse_remove(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_remove_st *state = ctx;

    if(state->last)
        art_node_free_f(art, state->last, state->dtor, state->ctx);

    state->last = node;

    return 0;
}

/// Destroy and remove node and all subnodes from ART.
///
/// \param art      ART
/// \param node     node to start with
/// \param prefix   prefix of nodes to remove, may be NULL
/// \param dtor     callback to destroy node data, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0                   success
/// \retval -1/E_ART_NOT_FOUND  \p prefix not found
static int art_node_remove(art_ct art, art_node_ct node, str_const_ct prefix, art_dtor_cb dtor, const void *ctx)
{
    art_remove_st state = { .dtor = dtor, .ctx = (void *)ctx };
    art_node_ct child, parent;
    unsigned char key;

    if(art_traverse(art, node, prefix, TRAV_POST, WITHOUT_KEY, FORWARD, art_traverse_remove, &state))
        return error_pass(), -1;

    child = state.last;

    while(1)
    {
        parent  = child->parent;
        key     = child->key;

        art_node_free_f(art, child, dtor, ctx);

        if(!parent)
        {
            art->root = NULL;
            break;
        }
        else
        {
            art_node_remove_child(parent, key, art->ordered);

            if(parent->size == 0) // failed merge
            {
                child = parent;
            }
            else if(parent->size == 1)
            {
                art_node_merge(art, parent);
                break;
            }
            else
            {
                if(parent->type > NODE4 && parent->size <= art_node_capacity(parent->type) / 4)
                    art_node_shrink(art, parent);

                break;
            }
        }
    }

    return 0;
}

void art_remove(art_ct art, art_node_ct node)
{
    assert_magic(art);
    assert_magic_n(&node->v.leaf, NODE_MAGIC);
    assert(art == node->v.leaf.art);

    art_node_remove(art, node, NO_PREFIX, NULL, NULL);
}

int art_remove_p(art_ct art, str_const_ct prefix)
{
    assert_magic(art);

    return error_pass_int(art_node_remove(art, art->root, prefix, NULL, NULL));
}

int art_remove_pf(art_ct art, str_const_ct prefix, art_dtor_cb dtor, const void *ctx)
{
    assert_magic(art);

    return error_pass_int(art_node_remove(art, art->root, prefix, dtor, ctx));
}

/// ART find state
typedef struct art_find_state
{
    art_pred_cb pred;   ///< predicate callback
    void        *ctx;   ///< predicate callback context
    art_node_ct node;   ///< matched node
} art_find_st;

/// ART traverse callback for finding leaf node.
///
/// \implements art_traverse_cb
///
/// \retval 0   node does not match
/// \retval 1   node does match
static int art_traverse_find(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_find_st *state = ctx;

    if(!state->pred(art, path, node->v.leaf.data, state->ctx))
        return 0;

    state->node = node;

    return 1;
}

/// Find first leaf node matching predicate.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param key      if true reconstruct node path
/// \param reverse  if true reverse traverse direction
/// \param pred     predicate to apply
/// \param ctx      \p pred context
///
/// \returns                        ART leaf node
/// \retval NULL/E_ART_NOT_FOUND    \p prefix not found or \p pred did not match any node
static art_node_ct art_find_generic(art_const_ct art, str_const_ct prefix, bool key, bool reverse, art_pred_cb pred, const void *ctx)
{
    art_find_st state = { .pred = pred, .ctx = (void *)ctx };
    int rc;

    assert_magic(art);
    assert(pred);

    if((rc = art_traverse((art_ct)art, art->root, prefix, TRAV_LEAF, key, reverse, art_traverse_find, &state)) < 0)
        return error_pass(), NULL;

    return_error_if_fail(rc == 1, E_ART_NOT_FOUND, NULL);

    return state.node;
}

art_node_ct art_find(art_const_ct art, art_pred_cb pred, const void *ctx)
{
    return error_pass_ptr(art_find_generic(art, NO_PREFIX, WITHOUT_KEY, FORWARD, pred, ctx));
}

void *art_find_data(art_const_ct art, art_pred_cb pred, const void *ctx)
{
    art_node_ct node;

    if(!(node = art_find(art, pred, ctx)))
        return error_pass(), NULL;

    return node->v.leaf.data;
}

art_node_ct art_find_k(art_const_ct art, art_pred_cb pred, const void *ctx)
{
    return error_pass_ptr(art_find_generic(art, NO_PREFIX, WITH_KEY, FORWARD, pred, ctx));
}

void *art_find_data_k(art_const_ct art, art_pred_cb pred, const void *ctx)
{
    art_node_ct node;

    if(!(node = art_find_k(art, pred, ctx)))
        return error_pass(), NULL;

    return node->v.leaf.data;
}

art_node_ct art_find_r(art_const_ct art, art_pred_cb pred, const void *ctx)
{
    return error_pass_ptr(art_find_generic(art, NO_PREFIX, WITHOUT_KEY, BACKWARD, pred, ctx));
}

void *art_find_data_r(art_const_ct art, art_pred_cb pred, const void *ctx)
{
    art_node_ct node;

    if(!(node = art_find_r(art, pred, ctx)))
        return error_pass(), NULL;

    return node->v.leaf.data;
}

art_node_ct art_find_rk(art_const_ct art, art_pred_cb pred, const void *ctx)
{
    return error_pass_ptr(art_find_generic(art, NO_PREFIX, WITH_KEY, BACKWARD, pred, ctx));
}

void *art_find_data_rk(art_const_ct art, art_pred_cb pred, const void *ctx)
{
    art_node_ct node;

    if(!(node = art_find_rk(art, pred, ctx)))
        return error_pass(), NULL;

    return node->v.leaf.data;
}

art_node_ct art_find_p(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx)
{
    return error_pass_ptr(art_find_generic(art, prefix, WITHOUT_KEY, FORWARD, pred, ctx));
}

void *art_find_data_p(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx)
{
    art_node_ct node;

    if(!(node = art_find_p(art, prefix, pred, ctx)))
        return error_pass(), NULL;

    return node->v.leaf.data;
}

art_node_ct art_find_pk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx)
{
    return error_pass_ptr(art_find_generic(art, prefix, WITH_KEY, FORWARD, pred, ctx));
}

void *art_find_data_pk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx)
{
    art_node_ct node;

    if(!(node = art_find_pk(art, prefix, pred, ctx)))
        return error_pass(), NULL;

    return node->v.leaf.data;
}

art_node_ct art_find_pr(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx)
{
    return error_pass_ptr(art_find_generic(art, prefix, WITHOUT_KEY, BACKWARD, pred, ctx));
}

void *art_find_data_pr(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx)
{
    art_node_ct node;

    if(!(node = art_find_pr(art, prefix, pred, ctx)))
        return error_pass(), NULL;

    return node->v.leaf.data;
}

art_node_ct art_find_prk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx)
{
    return error_pass_ptr(art_find_generic(art, prefix, WITH_KEY, BACKWARD, pred, ctx));
}

void *art_find_data_prk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, const void *ctx)
{
    art_node_ct node;

    if(!(node = art_find_prk(art, prefix, pred, ctx)))
        return error_pass(), NULL;

    return node->v.leaf.data;
}

/// ART fold state
typedef struct art_fold_state
{
    art_fold_cb fold;   ///< fold callback
    void        *ctx;   ///< fold callback context
} art_fold_st;

/// ART traverse callback for folding leaf nodes.
///
/// \implements art_traverse_cb
///
/// \retval >=0                 fold rc
/// \retval <0/E_ART_CALLBACK   fold error
static int art_traverse_fold(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_fold_st *state = ctx;

    return error_pack_int(E_ART_CALLBACK,
        state->fold(art, path, node->v.leaf.data, state->ctx));
}

/// Fold over all leaf nodes.
///
/// \param art      ART
/// \param prefix   prefix of nodes to search, may be NULL
/// \param key      if true reconstruct node path
/// \param reverse  if true reverse traverse direction
/// \param fold     callback to invoke on each leaf node
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval -1/E_ART_NOT_FOUND  \p prefix not found
/// \retval <0/E_ART_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
static int art_fold_generic(art_ct art, str_const_ct prefix, bool key, bool reverse, art_fold_cb fold, const void *ctx)
{
    art_fold_st state = { .fold = fold, .ctx = (void *)ctx };

    assert_magic(art);
    assert(fold);

    return error_pass_int(art_traverse(art, art->root, prefix, TRAV_LEAF, key, reverse, art_traverse_fold, &state));
}

int art_fold(art_ct art, art_fold_cb fold, const void *ctx)
{
    return error_pass_int(art_fold_generic(art, NO_PREFIX, WITHOUT_KEY, FORWARD, fold, ctx));
}

int art_fold_k(art_ct art, art_fold_cb fold, const void *ctx)
{
    return error_pass_int(art_fold_generic(art, NO_PREFIX, WITH_KEY, FORWARD, fold, ctx));
}

int art_fold_r(art_ct art, art_fold_cb fold, const void *ctx)
{
    return error_pass_int(art_fold_generic(art, NO_PREFIX, WITHOUT_KEY, BACKWARD, fold, ctx));
}

int art_fold_rk(art_ct art, art_fold_cb fold, const void *ctx)
{
    return error_pass_int(art_fold_generic(art, NULL, WITH_KEY, BACKWARD, fold, ctx));
}

int art_fold_p(art_ct art, str_const_ct prefix, art_fold_cb fold, const void *ctx)
{
    return error_pass_int(art_fold_generic(art, prefix, WITHOUT_KEY, FORWARD, fold, ctx));
}

int art_fold_pk(art_ct art, str_const_ct prefix, art_fold_cb fold, const void *ctx)
{
    return error_pass_int(art_fold_generic(art, prefix, WITH_KEY, FORWARD, fold, ctx));
}

int art_fold_pr(art_ct art, str_const_ct prefix, art_fold_cb fold, const void *ctx)
{
    return error_pass_int(art_fold_generic(art, prefix, WITHOUT_KEY, BACKWARD, fold, ctx));
}

int art_fold_prk(art_ct art, str_const_ct prefix, art_fold_cb fold, const void *ctx)
{
    return error_pass_int(art_fold_generic(art, prefix, WITH_KEY, BACKWARD, fold, ctx));
}

str_ct art_complete(art_const_ct art, str_const_ct prefix)
{
    art_node_ct node;
    str_ct lookup, path;
    size_t node_prefix = 0;

    assert_magic(art);

    return_error_if_fail(art->size, E_ART_EMPTY, NULL);

    if(prefix && !str_is_empty(prefix))
    {
        lookup  = art_path_new(prefix);
        node    = art_lookup(art, art->root, lookup, &node_prefix, NULL);
        return_error_if_fail(str_is_empty(lookup), E_ART_NOT_FOUND, NULL);
    }
    else
    {
        node = art->root;
    }

    if(!(path = str_dup_b(art_node_get_path(node) + node_prefix, node->path_len - node_prefix)))
        return error_wrap(), NULL;

    // nodes with 1 child may happen if a merge fails
    while(node->size == 1)
    {
        node = art_node_get_first_child(node);

        if(  !str_append_set(path, 1, node->key)
          || !str_append_b(path, art_node_get_path(node), node->path_len))
            return error_wrap(), str_unref(path), NULL;
    }

    return path;
}
