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

#include <ytil/con/art.h>
#include <ytil/ext/string.h>
#include <ytil/def.h>
#include <ytil/magic.h>
#include <ytil/simd.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef SIMD64
#   define NODE8_SIMD
#ifdef SIMD128
#   define NODE16_SIMD
#ifdef SIMD256
#   define NODE32_SIMD
#endif
#endif
#endif

#define WITH_KEY    true
#define WITHOUT_KEY false
#define FORWARD     false
#define BACKWARD    true
#define NO_PREFIX   NULL

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

enum def
{
      MAGIC      = define_magic('A', 'R', 'T')
    , NODE_MAGIC = define_magic('A', 'R', 'N')
};

typedef enum art_node_type
{
      LEAF
    , NODE4
    , NODE8
    , NODE16
    , NODE32
    , NODE64
    , NODE128
    , NODE256
} art_node_id;

typedef enum art_traverse_order
{
      TRAV_LEAF
    , TRAV_PRE
    , TRAV_IN
    , TRAV_POST
} art_traverse_id;

typedef union art_value
{
    struct leaf
    {
        DEBUG_MAGIC
        DEBUG_MEMBER(art_ct art)
        
        void *data;
    } leaf;
    
    struct node4
    {
        unsigned char key[4];
        art_node_ct child[4];
    } n4;
    
    struct node8
    {
        unsigned char key[8];
        art_node_ct child[8];
    } n8;
    
    struct node16
    {
        unsigned char key[16];
        art_node_ct child[16];
    } n16;
    
    struct node32
    {
        unsigned char key[32];
        art_node_ct child[32];
    } n32;
    
    struct node64
    {
        unsigned char key[256];
        art_node_ct child[64];
    } n64;
    
    struct node128
    {
        unsigned char key[256];
        art_node_ct child[128];
    } n128;
    
    struct node256
    {
        art_node_ct child[256];
    } n256;
} art_value_un;

typedef struct art_node
{
    uint8_t type;
    unsigned char key;
    uint16_t size;
    art_node_ct parent;
    uint32_t path_len;
    unsigned char *path;
    art_value_un v;
} art_node_st;

typedef struct art
{
    DEBUG_MAGIC
    
    art_node_ct root;
    size_t size;
    bool ordered;
} art_st;

typedef int (*art_traverse_cb)(art_ct art, art_node_ct node, str_ct path, void *ctx);

typedef struct art_traverse_state
{
    art_traverse_id order;
    str_ct path;
    bool reverse;
    art_traverse_cb cb;
    void *ctx;
} art_traverse_st;

typedef struct art_memsize_state
{
    art_size_cb sfun;
    void *ctx;
    size_t size;
} art_memsize_st;

typedef struct art_remove_state
{
    art_dtor_cb dtor;
    void *ctx;
    art_node_ct last;
} art_remove_st;

typedef struct art_find_state
{
    art_pred_cb pred;
    void *ctx;
    art_node_ct node;
} art_find_st;

typedef struct art_fold_state
{
    art_fold_cb fold;
    void *ctx;
} art_fold_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_ART_CALLBACK, "Callback error.")
    , ERROR_INFO(E_ART_EMPTY, "ART is empty.")
    , ERROR_INFO(E_ART_EXISTS, "Node already exists.")
    , ERROR_INFO(E_ART_INVALID_KEY, "Invalid key.")
    , ERROR_INFO(E_ART_NOT_FOUND, "Node not found.")
};


art_ct art_new(art_mode_id mode)
{
    art_ct art;
    
    if(!(art = calloc(1, sizeof(art_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic(art);
    
    art->ordered = mode == ART_MODE_ORDERED;
    
    return art;
}

static int art_node_remove(art_ct art, art_node_ct node, str_const_ct prefix, art_dtor_cb dtor, void *ctx);

void art_free(art_ct art)
{
    assert_magic(art);
    
    art_node_remove(art, art->root, NO_PREFIX, NULL, NULL);
    free(art);
}

void art_free_f(art_ct art, art_dtor_cb dtor, void *ctx)
{
    assert_magic(art);
    
    art_node_remove(art, art->root, NO_PREFIX, dtor, ctx);
    free(art);
}

art_ct art_free_if_empty(art_ct art)
{
    assert_magic(art);
    
    if(art->size)
        return art;
    
    art_free(art);
    
    return NULL;
}

void art_clear(art_ct art)
{
    assert_magic(art);
    
    art_node_remove(art, art->root, NO_PREFIX, NULL, NULL);
}

void art_clear_f(art_ct art, art_dtor_cb dtor, void *ctx)
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

static size_t art_node_size(art_node_id type)
{
    size_t size = sizeof(art_node_st) - sizeof(art_value_un);
    
    switch(type)
    {
    case LEAF:      return size + sizeof(struct leaf);
    case NODE4:     return size + sizeof(struct node4);
    case NODE8:     return size + sizeof(struct node8);
    case NODE16:    return size + sizeof(struct node16);
    case NODE32:    return size + sizeof(struct node32);
    case NODE64:    return size + sizeof(struct node64);
    case NODE128:   return size + sizeof(struct node128);
    case NODE256:   return size + sizeof(struct node256);
    default:        abort();
    }
}

static int art_traverse(art_ct art, art_node_ct node, str_const_ct prefix, art_traverse_id order, bool key, bool reverse, art_traverse_cb traverse, void *ctx);

static int art_traverse_memsize(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_memsize_st *state = ctx;
    
    state->size += art_node_size(node->type);
    
    if(node->path_len > sizeof(unsigned char*))
        state->size += node->path_len;
    
    if(node->type == LEAF && state->sfun)
        state->size += state->sfun(art, node->v.leaf.data, state->ctx);
    
    return 0;
}

size_t art_memsize(art_const_ct art)
{
    return art_memsize_f(art, NULL, NULL);
}

size_t art_memsize_f(art_const_ct art, art_size_cb sfun, void *ctx)
{
    art_memsize_st state = { .sfun = sfun, .ctx = ctx, .size = sizeof(art_st) };
    
    assert_magic(art);
    
    art_traverse((art_ct)art, art->root, NO_PREFIX, TRAV_PRE, WITHOUT_KEY, FORWARD, art_traverse_memsize, &state);
    
    return state.size;
}

static inline unsigned char *art_node_get_path(art_node_const_ct node)
{
    if(node->path_len <= sizeof(unsigned char*))
        return (unsigned char*)&node->path;
    else
        return node->path;
}

static void art_node_drop_path(art_node_ct node, size_t prefix)
{
    size_t len = node->path_len - prefix;
    unsigned char *path;
    
    if(node->path_len <= sizeof(unsigned char*))
    {
        path = (unsigned char*)&node->path;
        memcpy(path, &path[prefix], len);
    }
    else if(len <= sizeof(unsigned char*))
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

static int art_node_prepend_path(art_node_ct node, const unsigned char *prefix, size_t len, unsigned char key)
{
    unsigned char *path;
    
    if(node->path_len+len+1 <= sizeof(unsigned char*))
    {
        path = art_node_get_path(node);
        memmove(&path[len+1], path, node->path_len);
        memcpy(path, prefix, len);
        path[len] = key;
    }
    else
    {
        if(!(path = malloc(node->path_len+len+1)))
            return error_wrap_errno(malloc), -1;
        
        memcpy(path, prefix, len);
        path[len] = key;
        memcpy(&path[len+1], art_node_get_path(node), node->path_len);
        
        if(node->path_len > sizeof(unsigned char*))
            free(node->path);
        
        node->path = path;
    }
    
    node->path_len += len+1;
    
    return 0;
}

void *art_node_get_data(art_node_const_ct node)
{
    assert_magic_n(&node->v.leaf, NODE_MAGIC);
    
    return node->v.leaf.data;
}

void art_node_set_data(art_node_ct node, const void *data)
{
    assert_magic_n(&node->v.leaf, NODE_MAGIC);
    
    node->v.leaf.data = (void*)data;
}

str_ct art_node_get_key(art_node_const_ct node)
{
    str_ct key;
    
    assert_magic_n(&node->v.leaf, NODE_MAGIC);
    
    if(!(key = str_dup_b(art_node_get_path(node), node->path_len)))
        return error_wrap(), NULL;
    
    for(; node->parent; node = node->parent)
        if(!str_prepend_b(key, &node->key, 1)
        || !str_prepend_b(key, art_node_get_path(node->parent), node->parent->path_len))
            return error_wrap(), str_unref(key), NULL;
    
    return key;
}

static inline size_t art_node_get_common_prefix(art_node_const_ct node, str_const_ct key)
{
    return memprefix(art_node_get_path(node), str_bc(key), MIN(node->path_len, str_len(key)));
}

static int art_node_find_child(const unsigned char *keys, size_t size, unsigned char key)
{
    size_t left = 0, right = size, pos;
    
    while(left < right)
    {
        pos = left + (right-left) / 2;
        
        if(key == keys[pos])
            return pos;
        
        if(key < keys[pos])
            right = pos;
        else
            left = pos+1;
    }
    
    return -1 - left; // negative insert position -1
}

static inline art_node_ct *art_node4_get_child(art_node_const_ct node, unsigned char key)
{
    size_t i;
    
    for(i=0; i < node->size; i++)
        if(node->v.n4.key[i] == key)
            return (art_node_ct*)&node->v.n4.child[i];
    
    return NULL;
}

static inline art_node_ct *art_node8_get_child(art_node_const_ct node, unsigned char key)
{
#ifdef NODE8_SIMD
    int index = simd64_index8(node->v.n8.key, node->size, key);
#else
    int index = art_node_find_child(node->v.n8.key, node->size, key);
#endif
    return index < 0 ? NULL : (art_node_ct*)&node->v.n8.child[index];
}

static inline art_node_ct *art_node16_get_child(art_node_const_ct node, unsigned char key)
{
#ifdef NODE16_SIMD
    int index = simd128_index8(node->v.n16.key, node->size, key);
#else
    int index = art_node_find_child(node->v.n16.key, node->size, key);
#endif
    return index < 0 ? NULL : (art_node_ct*)&node->v.n16.child[index];
}

static inline art_node_ct *art_node32_get_child(art_node_const_ct node, unsigned char key)
{
#ifdef NODE32_SIMD
    int index = simd256_index8(node->v.n32.key, node->size, key);
#else
    int index = art_node_find_child(node->v.n32.key, node->size, key);
#endif
    return index < 0 ? NULL : (art_node_ct*)&node->v.n32.child[index];
}

static inline art_node_ct *art_node64_get_child(art_node_const_ct node, unsigned char key)
{
    uint8_t index = node->v.n64.key[key];
    
    return index == 0xff ? NULL : (art_node_ct*)&node->v.n64.child[index];
}

static inline art_node_ct *art_node128_get_child(art_node_const_ct node, unsigned char key)
{
    uint8_t index = node->v.n128.key[key];
    
    return index == 0xff ? NULL : (art_node_ct*)&node->v.n128.child[index];
}

static inline art_node_ct *art_node256_get_child(art_node_const_ct node, unsigned char key)
{
    return node->v.n256.child[key] ? (art_node_ct*)&node->v.n256.child[key] : NULL;
}

static art_node_ct *art_node_get_child(art_node_const_ct node, unsigned char key)
{
    switch(node->type)
    {
    case NODE4:     return art_node4_get_child(node, key);
    case NODE8:     return art_node8_get_child(node, key);
    case NODE16:    return art_node16_get_child(node, key);
    case NODE32:    return art_node32_get_child(node, key);
    case NODE64:    return art_node64_get_child(node, key);
    case NODE128:   return art_node128_get_child(node, key);
    case NODE256:   return art_node256_get_child(node, key);
    default:        abort();
    }
}

static art_node_ct art_lookup(art_const_ct art, art_node_const_ct node, str_ct key, size_t *prefix, art_node_ct **ref)
{
    art_node_ct *child, *node_ref;
    size_t prefixlen;
    
    return_value_if_fail(node, NULL);
    
    if(ref)
    {
        if(node == art->root)
            *ref = (art_node_ct*)&art->root;
        else
            *ref = art_node_get_child(node->parent, node->key);
    }
    
    prefix = prefix ? prefix : &prefixlen;
    ref = ref ? ref : &node_ref;
    
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
        node = *child;
        *ref = child;
    }
}

// setup transient path from key
// terminator is part of string keys
#define art_path_new(key) \
    tstr_new_bs(str_bc(key), str_len(key) + (str_is_binary(key) ? 0 : 1))

art_node_ct art_get(art_const_ct art, str_const_ct key)
{
    art_node_ct node;
    str_ct path;
    
    assert_magic(art);
    
    path = art_path_new(key);
    
    if(!(node = art_lookup(art, art->root, path, NULL, NULL))
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

static art_node_ct art_node_new(art_node_id type, unsigned char key, art_node_ct parent, str_const_ct path)
{
    art_node_ct node;
    
    if(!(node = calloc(1, art_node_size(type))))
        return error_wrap_errno(calloc), NULL;
    
    if(path)
    {
        if(str_len(path) <= sizeof(unsigned char*))
            memcpy(&node->path, str_bc(path), str_len(path));
        else if(!(node->path = memdup(str_bc(path), str_len(path))))
            return error_wrap_errno(memdup), NULL;
    }
    
    switch(type)
    {
    case NODE64:    memset(node->v.n64.key, 0xff, sizeof(node->v.n64.key)); break;
    case NODE128:   memset(node->v.n128.key, 0xff, sizeof(node->v.n128.key)); break;
    default:        break;
    }
    
    node->type = type;
    node->path_len = path ? str_len(path) : 0;
    node->key = key;
    node->parent = parent;
    
    return node;
}

static art_node_ct art_node_new_leaf(art_ct art, str_const_ct path, void *data)
{
    art_node_ct node;
    
    if(!(node = art_node_new(LEAF, '\0', NULL, path)))
        return error_pass(), NULL;
    
    init_magic_n(&node->v.leaf, NODE_MAGIC);
    DEBUG(node->v.leaf.art = art);
    node->v.leaf.data = data;
    
    return node;
}

static inline size_t art_node_capacity(art_node_id type)
{
    switch(type)
    {
    case NODE4:     return 4;
    case NODE8:     return 8;
    case NODE16:    return 16;
    case NODE32:    return 32;
    case NODE64:    return 64;
    case NODE128:   return 128;
    case NODE256:   return 256;
    default:        abort();
    }
}

static inline void art_node_copy_list(art_node_ct node2, unsigned char *keys2, art_node_ct *children2, art_node_ct node1, unsigned char *keys1, art_node_ct *children1)
{
    size_t n;
    
    memcpy(keys2, keys1, node1->size);
    memcpy(children2, children1, node1->size * sizeof(art_node_ct));
    
    for(n=0; n < node1->size; n++)
        children1[n]->parent = node2;
}

static inline void art_node4_to_node8(art_node_ct n8, art_node_ct n4)
{
    art_node_copy_list(n8, n8->v.n8.key, n8->v.n8.child, n4, n4->v.n4.key, n4->v.n4.child);
}

static inline void art_node8_to_node16(art_node_ct n16, art_node_ct n8)
{
    art_node_copy_list(n16, n16->v.n16.key, n16->v.n16.child, n8, n8->v.n8.key, n8->v.n8.child);
}

static inline void art_node16_to_node32(art_node_ct n32, art_node_ct n16)
{
    art_node_copy_list(n32, n32->v.n32.key, n32->v.n32.child, n16, n16->v.n16.key, n16->v.n16.child);
}

static inline void art_node32_to_node64(art_node_ct n64, art_node_ct n32)
{
    size_t n;
    
    for(n=0; n < n32->size; n++)
    {
        n64->v.n64.key[n32->v.n32.key[n]] = n;
        n64->v.n64.child[n] = n32->v.n32.child[n];
        
        n64->v.n64.child[n]->parent = n64;
    }
}

static inline void art_node64_to_node128(art_node_ct n128, art_node_ct n64)
{
    size_t n;
    
    memcpy(n128->v.n128.key, n64->v.n64.key, 256);
    memcpy(n128->v.n128.child, n64->v.n64.child, n64->size * sizeof(art_node_ct));
    
    for(n=0; n < n64->size; n++)
        n64->v.n64.child[n]->parent = n128;
}

static inline void art_node128_to_node256(art_node_ct n256, art_node_ct n128)
{
    size_t n;
    
    for(n=0; n < 256; n++)
        if(n128->v.n128.key[n] != 0xff)
        {
            n256->v.n256.child[n] = n128->v.n128.child[n128->v.n128.key[n]];
            n256->v.n256.child[n]->parent = n256;
        }
}

static art_node_ct art_node_grow(art_node_ct *ref)
{
    art_node_ct node1 = *ref, node2;
    
    if(!(node2 = art_node_new(node1->type+1, node1->key, node1->parent, NULL)))
        return error_pass(), NULL;
    
    switch(node1->type)
    {
    case NODE4:     art_node4_to_node8(node2, node1); break;
    case NODE8:     art_node8_to_node16(node2, node1); break;
    case NODE16:    art_node16_to_node32(node2, node1); break;
    case NODE32:    art_node32_to_node64(node2, node1); break;
    case NODE64:    art_node64_to_node128(node2, node1); break;
    case NODE128:   art_node128_to_node256(node2, node1); break;
    default:        abort();
    }
    
    node2->size = node1->size;
    node2->path = node1->path;
    node2->path_len = node1->path_len;
    
    *ref = node2;
    free(node1);
    
    return node2;
}

static inline void art_node_insert_sorted(art_node_ct parent, unsigned char *keys, art_node_ct *children, unsigned char key, art_node_ct child, int index)
{
    memmove(&keys[index+1], &keys[index], parent->size-index);
    memmove(&children[index+1], &children[index], (parent->size-index)*sizeof(art_node_ct));
    
    keys[index] = key;
    children[index] = child;
}

#if defined(NODE8_SIMD) || defined(NODE16_SIMD) || defined(NODE32_SIMD)
static inline void art_node_insert_unsorted(art_node_ct parent, unsigned char *keys, art_node_ct *children, unsigned char key, art_node_ct child)
{
    keys[parent->size] = key;
    children[parent->size] = child;
}
#endif

static inline void art_node_insert_index(art_node_ct parent, unsigned char *keys, art_node_ct *children, unsigned char key, art_node_ct child)
{
    keys[key] = parent->size;
    children[parent->size] = child;
}

static inline void art_node4_insert(art_node_ct parent, unsigned char key, art_node_ct child)
{
    size_t k;
    
    for(k=0; k < parent->size; k++)
        if(key < parent->v.n4.key[k])
            break;
    
    art_node_insert_sorted(parent, parent->v.n4.key, parent->v.n4.child, key, child, k);
}

static inline void art_node8_insert(art_node_ct parent, unsigned char key, art_node_ct child, bool ordered)
{
#ifdef NODE8_SIMD
    if(!ordered)
        art_node_insert_unsorted(parent, parent->v.n8.key, parent->v.n8.child, key, child);
    else
#endif
        art_node_insert_sorted(parent, parent->v.n8.key, parent->v.n8.child, key, child,
            -art_node_find_child(parent->v.n8.key, parent->size, key) -1);
}

static inline void art_node16_insert(art_node_ct parent, unsigned char key, art_node_ct child, bool ordered)
{
#ifdef NODE16_SIMD
    if(!ordered)
        art_node_insert_unsorted(parent, parent->v.n16.key, parent->v.n16.child, key, child);
    else
#endif
        art_node_insert_sorted(parent, parent->v.n16.key, parent->v.n16.child, key, child,
            -art_node_find_child(parent->v.n16.key, parent->size, key) -1);
}

static inline void art_node32_insert(art_node_ct parent, unsigned char key, art_node_ct child, bool ordered)
{
#ifdef NODE32_SIMD
    if(!ordered)
        art_node_insert_unsorted(parent, parent->v.n32.key, parent->v.n32.child, key, child);
    else
#endif
        art_node_insert_sorted(parent, parent->v.n32.key, parent->v.n32.child, key, child,
            -art_node_find_child(parent->v.n32.key, parent->size, key) -1);
}

static inline void art_node64_insert(art_node_ct parent, unsigned char key, art_node_ct child)
{
    art_node_insert_index(parent, parent->v.n64.key, parent->v.n64.child, key, child);
}

static inline void art_node128_insert(art_node_ct parent, unsigned char key, art_node_ct child)
{
    art_node_insert_index(parent, parent->v.n128.key, parent->v.n128.child, key, child);
}

static inline void art_node256_insert(art_node_ct parent, unsigned char key, art_node_ct child)
{
    parent->v.n256.child[key] = child;
}

static art_node_ct art_node_insert(art_node_ct parent, unsigned char key, art_node_ct child, bool ordered)
{
    switch(parent->type)
    {
    case NODE4:     art_node4_insert(parent, key, child); break;
    case NODE8:     art_node8_insert(parent, key, child, ordered); break;
    case NODE16:    art_node16_insert(parent, key, child, ordered); break;
    case NODE32:    art_node32_insert(parent, key, child, ordered); break;
    case NODE64:    art_node64_insert(parent, key, child); break;
    case NODE128:   art_node128_insert(parent, key, child); break;
    case NODE256:   art_node256_insert(parent, key, child); break;
    default:        abort();
    }
    
    parent->size++;
    child->key = key;
    child->parent = parent;
    
    return child;
}

static art_node_ct art_node_split(art_ct art, art_node_ct *ref, size_t prefix)
{
    art_node_ct child = *ref, inode;
    unsigned char *path = art_node_get_path(child);
    
    assert(prefix+1 <= child->path_len);
    
    if(!(inode = art_node_new(NODE4, child->key, child->parent, tstr_new_bs(path, prefix))))
        return error_pass(), NULL;
    
    *ref = inode;
    
    art_node_insert(inode, path[prefix], child, art->ordered);
    art_node_drop_path(child, prefix+1);
    
    return inode;
}

static void art_node_free(art_node_ct node)
{
    if(node->path_len > sizeof(unsigned char*))
        free(node->path);
    
    free(node);
}

art_node_ct art_insert(art_ct art, str_const_ct key, void *data)
{
    art_node_ct leaf, node, *node_ref;
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
    
    node = art_lookup(art, art->root, path, &prefix_len, &node_ref);
    
    if(str_is_empty(path)
    || (prefix_len == node->path_len && node->type == LEAF))
        return error_set(E_ART_EXISTS), NULL;
    
    leaf_key = str_first_u(path);
    str_cut_head(path, 1);
    
    if(!(leaf = art_node_new_leaf(art, path, data)))
        return error_pass(), NULL;
    
    if(prefix_len < node->path_len)
    {
        if(!(node = art_node_split(art, node_ref, prefix_len)))
            return error_pass(), art_node_free(leaf), NULL;
    }
    else
    {
        if(node->size == art_node_capacity(node->type)
        && !(node = art_node_grow(node_ref)))
            return error_pass(), art_node_free(leaf), NULL;
    }
    
    art->size++;
    
    return art_node_insert(node, leaf_key, leaf, art->ordered);
}

static inline void art_node_remove_child_sorted(art_node_ct node, unsigned char *keys, art_node_ct *children, size_t index)
{
    if(index+1 < node->size)
    {
        memmove(&keys[index], &keys[index+1], node->size-index-1);
        memmove(&children[index], &children[index+1], (node->size-index-1)*sizeof(art_node_ct));
    }
}

#if defined(NODE8_SIMD) || defined(NODE16_SIMD) || defined(NODE32_SIMD)
static inline void art_node_remove_child_unsorted(art_node_ct node, unsigned char *keys, art_node_ct *children, size_t index)
{
    if(index+1 < node->size)
    {
        keys[index] = keys[node->size-1];
        children[index] = children[node->size-1];
    }
}
#endif

static inline void art_node_remove_child_index(art_node_ct node, unsigned char *keys, art_node_ct *children, unsigned char key)
{
    art_node_ct last;
    size_t i;
    
    if(node->size > 1)
    {
        last = children[node->size-1];
        
        for(i=0; i < 256; i++)
            if(keys[i] != 0xff && children[keys[i]] == last)
            {
                keys[i] = keys[key];
                children[keys[i]] = last;
                break;
            }
    }
    
    keys[key] = 0xff;
}

static inline void art_node4_remove_child(art_node_ct parent, unsigned char key)
{
    size_t i;
    
    for(i=0; i < parent->size; i++)
        if(parent->v.n4.key[i] == key)
        {
            art_node_remove_child_sorted(parent, parent->v.n4.key, parent->v.n4.child, i);
            return;
        }
}

static inline void art_node8_remove_child(art_node_ct parent, unsigned char key, bool ordered)
{
#ifdef NODE8_SIMD
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

static inline void art_node16_remove_child(art_node_ct parent, unsigned char key, bool ordered)
{
#ifdef NODE16_SIMD
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

static inline void art_node32_remove_child(art_node_ct parent, unsigned char key, bool ordered)
{
#ifdef NODE32_SIMD
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

static inline void art_node64_remove_child(art_node_ct parent, unsigned char key)
{
    art_node_remove_child_index(parent, parent->v.n64.key, parent->v.n64.child, key);
}

static inline void art_node128_remove_child(art_node_ct parent, unsigned char key)
{
    art_node_remove_child_index(parent, parent->v.n128.key, parent->v.n128.child, key);
}

static inline void art_node256_remove_child(art_node_ct parent, unsigned char key)
{
    parent->v.n256.child[key] = NULL;
}

static void art_node_remove_child(art_node_ct parent, unsigned char key, bool ordered)
{
    switch(parent->type)
    {
    case NODE4:     art_node4_remove_child(parent, key); break;
    case NODE8:     art_node8_remove_child(parent, key, ordered); break;
    case NODE16:    art_node16_remove_child(parent, key, ordered); break;
    case NODE32:    art_node32_remove_child(parent, key, ordered); break;
    case NODE64:    art_node64_remove_child(parent, key); break;
    case NODE128:   art_node128_remove_child(parent, key); break;
    case NODE256:   art_node256_remove_child(parent, key); break;
    default:        abort();
    }
    
    parent->size--;
}

static inline void art_node8_to_node4(art_node_ct n4, art_node_ct n8)
{
    art_node_copy_list(n4, n4->v.n4.key, n4->v.n4.child, n8, n8->v.n8.key, n8->v.n8.child);
}

static inline void art_node16_to_node8(art_node_ct n8, art_node_ct n16)
{
    art_node_copy_list(n8, n8->v.n8.key, n8->v.n8.child, n16, n16->v.n16.key, n16->v.n16.child);
}

static inline void art_node32_to_node16(art_node_ct n16, art_node_ct n32)
{
    art_node_copy_list(n16, n16->v.n16.key, n16->v.n16.child, n32, n32->v.n32.key, n32->v.n32.child);
}

static inline void art_node64_to_node32(art_node_ct n32, art_node_ct n64)
{
    size_t n, s;
    
    for(n=0, s=0; n < 256 && s < n64->size; n++)
        if(n64->v.n64.key[n] != 0xff)
        {
            n32->v.n32.key[s] = n;
            n32->v.n32.child[s] = n64->v.n64.child[n64->v.n64.key[n]];
            n32->v.n32.child[s]->parent = n32;
            s++;
        }
}

static inline void art_node128_to_node64(art_node_ct n64, art_node_ct n128)
{
    size_t n;
    
    memcpy(n64->v.n64.key, n128->v.n128.key, 256);
    memcpy(n64->v.n64.child, n128->v.n128.child, n128->size * sizeof(art_node_ct));
    
    for(n=0; n < n128->size; n++)
        n128->v.n128.child[n]->parent = n64;
}

static inline void art_node256_to_node128(art_node_ct n128, art_node_ct n256)
{
    size_t n, s;
    
    for(n=0, s=0; n < 256 && s < n256->size; n++)
        if(n256->v.n256.child[n])
        {
            n128->v.n128.key[n] = s;
            n128->v.n128.child[s] = n256->v.n256.child[n];
            n128->v.n128.child[s]->parent = n128;
            s++;
        }
}

static art_node_ct art_node_shrink(art_ct art, art_node_ct node1)
{
    art_node_ct node2;
    
    if(!(node2 = art_node_new(node1->type-1, node1->key, node1->parent, NULL)))
        return error_pass(), NULL;
    
    switch(node1->type)
    {
    case NODE8:     art_node8_to_node4(node2, node1); break;
    case NODE16:    art_node16_to_node8(node2, node1); break;
    case NODE32:    art_node32_to_node16(node2, node1); break;
    case NODE64:    art_node64_to_node32(node2, node1); break;
    case NODE128:   art_node128_to_node64(node2, node1); break;
    case NODE256:   art_node256_to_node128(node2, node1); break;
    default:        abort();
    }
    
    node2->size = node1->size;
    node2->path = node1->path;
    node2->path_len = node1->path_len;
    
    if(node1 == art->root)
        art->root = node2;
    else
        *art_node_get_child(node1->parent, node1->key) = node2;
    
    free(node1);
    
    return node2;
}

static int art_traverse_first(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_node_ct *child = ctx;
    
    *child = node;
    
    return 1;
}

static art_node_ct art_node_get_first_child(art_node_ct node)
{
    art_node_ct child = NULL;
    
    art_traverse(NULL, node, NO_PREFIX, TRAV_POST, WITHOUT_KEY, FORWARD, art_traverse_first, &child);
    
    return child;
}

static art_node_ct art_node_merge(art_ct art, art_node_ct node)
{
    art_node_ct child;
    
    // node should be NODE4, but if shrink failed we may have any other type
    child = art_node_get_first_child(node);
    
    if(art_node_prepend_path(child, art_node_get_path(node), node->path_len, child->key))
        return error_pass(), NULL;
    
    child->parent = node->parent;
    child->key = node->key;
    
    if(node == art->root)
        art->root = child;
    else
        *art_node_get_child(node->parent, node->key) = child;
    
    art_node_free(node);
    
    return child;
}

static void art_node_free_f(art_ct art, art_node_ct node, art_dtor_cb dtor, void *ctx)
{
    if(node->type == LEAF)
    {
        if(dtor)
            dtor(art, node->v.leaf.data, ctx);
        
        art->size--;
    }
    
    art_node_free(node);
}

static int art_traverse_remove(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_remove_st *state = ctx;
    
    if(state->last)
        art_node_free_f(art, state->last, state->dtor, state->ctx);
    
    state->last = node;
    
    return 0;
}

static int art_node_remove(art_ct art, art_node_ct node, str_const_ct prefix, art_dtor_cb dtor, void *ctx)
{
    art_remove_st state = { .dtor = dtor, .ctx = ctx };
    art_node_ct child, parent;
    unsigned char key;
    
    if(art_traverse(art, node, prefix, TRAV_POST, WITHOUT_KEY, FORWARD, art_traverse_remove, &state))
        return error_pass(), -1;
    
    child = state.last;
    
    while(1)
    {
        parent = child->parent;
        key = child->key;
        
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
                if(parent->type > NODE4 && parent->size <= art_node_capacity(parent->type)/4)
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

int art_remove_pf(art_ct art, str_const_ct prefix, art_dtor_cb dtor, void *ctx)
{
    assert_magic(art);
    
    return error_pass_int(art_node_remove(art, art->root, prefix, dtor, ctx));
}

static int art_traverse_node(art_ct art, size_t pos, art_node_ct node, art_traverse_st *state);

static int art_traverse_list(art_ct art, art_node_ct node, unsigned char *keys, art_node_ct *children, size_t size, art_traverse_st *state)
{
    art_node_ct child;
    size_t i;
    int rc;
    
    for(i=0; i < size; i++)
    {
        child = children[state->reverse ? size-i-1 : i];
        
        if((rc = art_traverse_node(art, i, child, state)))
            return rc;
    }
    
    return 0;
}

static int art_traverse_index(art_ct art, art_node_ct node, unsigned char *keys, art_node_ct *children, size_t size, art_traverse_st *state)
{
    unsigned char key;
    size_t i, pos;
    int rc;
    
    for(i=0, pos=0; pos < size; i++)
        if((key = keys[state->reverse ? 255-i : i]) != 0xff)
        {
            if((rc = art_traverse_node(art, pos, children[key], state)))
                return rc;
            
            pos++;
        }
    
    return 0;
}

static inline int art_traverse_node4(art_ct art, art_node_ct node, art_traverse_st *state)
{
    return art_traverse_list(art, node, node->v.n4.key, node->v.n4.child, node->size, state);
}

static inline int art_traverse_node8(art_ct art, art_node_ct node, art_traverse_st *state)
{
    return art_traverse_list(art, node, node->v.n8.key, node->v.n8.child, node->size, state);
}

static inline int art_traverse_node16(art_ct art, art_node_ct node, art_traverse_st *state)
{
    return art_traverse_list(art, node, node->v.n16.key, node->v.n16.child, node->size, state);
}

static inline int art_traverse_node32(art_ct art, art_node_ct node, art_traverse_st *state)
{
    return art_traverse_list(art, node, node->v.n32.key, node->v.n32.child, node->size, state);
}

static inline int art_traverse_node64(art_ct art, art_node_ct node, art_traverse_st *state)
{
    return art_traverse_index(art, node, node->v.n64.key, node->v.n64.child, node->size, state);
}

static inline int art_traverse_node128(art_ct art, art_node_ct node, art_traverse_st *state)
{
    return art_traverse_index(art, node, node->v.n128.key, node->v.n128.child, node->size, state);
}

static inline int art_traverse_node256(art_ct art, art_node_ct node, art_traverse_st *state)
{
    art_node_ct child;
    size_t i, pos;
    int rc;
    
    for(i=0, pos=0; pos < node->size; i++)
        if((child = node->v.n256.child[state->reverse ? 255-i : i]))
        {
            if((rc = art_traverse_node(art, pos, child, state)))
                return rc;
            
            pos++;
        }
    
    return 0;
}

static int art_traverse_node(art_ct art, size_t pos, art_node_ct node, art_traverse_st *state)
{
    size_t path_len = node->path_len;
    int rc = 0;
    
    if(state->order == TRAV_IN && node->parent && pos > 0
    && (rc = error_pass_int(state->cb(art, node->parent, state->path, state->ctx))))
        return rc;
    
    if(state->path)
    {
        if((node->parent && !str_append_set(state->path, 1, node->key))
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
            switch(node->type)
            {
            case NODE4:     rc = art_traverse_node4(art, node, state); break;
            case NODE8:     rc = art_traverse_node8(art, node, state); break;
            case NODE16:    rc = art_traverse_node16(art, node, state); break;
            case NODE32:    rc = art_traverse_node32(art, node, state); break;
            case NODE64:    rc = art_traverse_node64(art, node, state); break;
            case NODE128:   rc = art_traverse_node128(art, node, state); break;
            case NODE256:   rc = art_traverse_node256(art, node, state); break;
            default:        abort();
            }
        
        if(!rc && state->order == TRAV_POST)
            rc = error_pass_int(state->cb(art, node, state->path, state->ctx));
    }
    
    if(state->path)
        str_cut_tail(state->path, path_len);
    
    return rc;
}

static int art_traverse(art_ct art, art_node_ct node, str_const_ct prefix, art_traverse_id order, bool key, bool reverse, art_traverse_cb traverse, void *ctx)
{
    art_traverse_st state = { .order = order, .reverse = reverse, .cb = traverse, .ctx = ctx };
    str_ct lookup;
    size_t node_prefix;
    int rc;
    
    return_error_if_fail(node, E_ART_NOT_FOUND, -1);
    
    if(prefix)
    {
        lookup = art_path_new(prefix);
        node = art_lookup(art, node, lookup, &node_prefix, NULL);
        return_error_if_fail(str_is_empty(lookup), E_ART_NOT_FOUND, -1);
        
        if(node->parent)
            node_prefix++; // dont dup node->key
        
        if(key && !(state.path = str_dup_n(prefix, str_len(prefix)-node_prefix)))
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

static int art_traverse_find(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_find_st *state = ctx;
    
    if(!state->pred(art, path, node->v.leaf.data, state->ctx))
        return 0;
    
    state->node = node;
    
    return 1;
}

static art_node_ct _art_find(art_const_ct art, str_const_ct prefix, bool key, bool reverse, art_pred_cb pred, void *ctx)
{
    art_find_st state = { .pred = pred, .ctx = ctx };
    int rc;
    
    assert_magic(art);
    assert(pred);
    
    if((rc = art_traverse((art_ct)art, art->root, prefix, TRAV_LEAF, key, reverse, art_traverse_find, &state)) < 0)
        return error_pass(), NULL;
    
    return_error_if_fail(rc == 1, E_ART_NOT_FOUND, NULL);
    
    return state.node;
}

art_node_ct art_find(art_const_ct art, art_pred_cb pred, void *ctx)
{
    return error_pass_ptr(_art_find(art, NO_PREFIX, WITHOUT_KEY, FORWARD, pred, ctx));
}

void *art_find_data(art_const_ct art, art_pred_cb pred, void *ctx)
{
    art_node_ct node;
    
    if(!(node = art_find(art, pred, ctx)))
        return error_pass(), NULL;
    
    return node->v.leaf.data;
}

art_node_ct art_find_k(art_const_ct art, art_pred_cb pred, void *ctx)
{
    return error_pass_ptr(_art_find(art, NO_PREFIX, WITH_KEY, FORWARD, pred, ctx));
}

void *art_find_data_k(art_const_ct art, art_pred_cb pred, void *ctx)
{
    art_node_ct node;
    
    if(!(node = art_find_k(art, pred, ctx)))
        return error_pass(), NULL;
    
    return node->v.leaf.data;
}

art_node_ct art_find_r(art_const_ct art, art_pred_cb pred, void *ctx)
{
    return error_pass_ptr(_art_find(art, NO_PREFIX, WITHOUT_KEY, BACKWARD, pred, ctx));
}

void *art_find_data_r(art_const_ct art, art_pred_cb pred, void *ctx)
{
    art_node_ct node;
    
    if(!(node = art_find_r(art, pred, ctx)))
        return error_pass(), NULL;
    
    return node->v.leaf.data;
}

art_node_ct art_find_rk(art_const_ct art, art_pred_cb pred, void *ctx)
{
    return error_pass_ptr(_art_find(art, NO_PREFIX, WITH_KEY, BACKWARD, pred, ctx));
}

void *art_find_data_rk(art_const_ct art, art_pred_cb pred, void *ctx)
{
    art_node_ct node;
    
    if(!(node = art_find_rk(art, pred, ctx)))
        return error_pass(), NULL;
    
    return node->v.leaf.data;
}

art_node_ct art_find_p(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx)
{
    return error_pass_ptr(_art_find(art, prefix, WITHOUT_KEY, FORWARD, pred, ctx));
}

void *art_find_data_p(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx)
{
    art_node_ct node;
    
    if(!(node = art_find_p(art, prefix, pred, ctx)))
        return error_pass(), NULL;
    
    return node->v.leaf.data;
}

art_node_ct art_find_pk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx)
{
    return error_pass_ptr(_art_find(art, prefix, WITH_KEY, FORWARD, pred, ctx));
}

void *art_find_data_pk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx)
{
    art_node_ct node;
    
    if(!(node = art_find_pk(art, prefix, pred, ctx)))
        return error_pass(), NULL;
    
    return node->v.leaf.data;
}

art_node_ct art_find_pr(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx)
{
    return error_pass_ptr(_art_find(art, prefix, WITHOUT_KEY, BACKWARD, pred, ctx));
}

void *art_find_data_pr(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx)
{
    art_node_ct node;
    
    if(!(node = art_find_pr(art, prefix, pred, ctx)))
        return error_pass(), NULL;
    
    return node->v.leaf.data;
}

art_node_ct art_find_prk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx)
{
    return error_pass_ptr(_art_find(art, prefix, WITH_KEY, BACKWARD, pred, ctx));
}

void *art_find_data_prk(art_const_ct art, str_const_ct prefix, art_pred_cb pred, void *ctx)
{
    art_node_ct node;
    
    if(!(node = art_find_prk(art, prefix, pred, ctx)))
        return error_pass(), NULL;
    
    return node->v.leaf.data;
}

static int art_traverse_fold(art_ct art, art_node_ct node, str_ct path, void *ctx)
{
    art_fold_st *state = ctx;
    
    return error_push_int(E_ART_CALLBACK,
        state->fold(art, path, node->v.leaf.data, state->ctx));
}

static int _art_fold(art_ct art, str_const_ct prefix, bool key, bool reverse, art_fold_cb fold, void *ctx)
{
    art_fold_st state = { .fold = fold, .ctx = ctx };
    
    assert_magic(art);
    assert(fold);
    
    return error_pass_int(art_traverse(art, art->root, prefix, TRAV_LEAF, key, reverse, art_traverse_fold, &state));
}

int art_fold(art_ct art, art_fold_cb fold, void *ctx)
{
    return error_pass_int(_art_fold(art, NO_PREFIX, WITHOUT_KEY, FORWARD, fold, ctx));
}

int art_fold_k(art_ct art, art_fold_cb fold, void *ctx)
{
    return error_pass_int(_art_fold(art, NO_PREFIX, WITH_KEY, FORWARD, fold, ctx));
}

int art_fold_r(art_ct art, art_fold_cb fold, void *ctx)
{
    return error_pass_int(_art_fold(art, NO_PREFIX, WITHOUT_KEY, BACKWARD, fold, ctx));
}

int art_fold_rk(art_ct art, art_fold_cb fold, void *ctx)
{
    return error_pass_int(_art_fold(art, NULL, WITH_KEY, BACKWARD, fold, ctx));
}

int art_fold_p(art_ct art, str_const_ct prefix, art_fold_cb fold, void *ctx)
{
    return error_pass_int(_art_fold(art, prefix, WITHOUT_KEY, FORWARD, fold, ctx));
}

int art_fold_pk(art_ct art, str_const_ct prefix, art_fold_cb fold, void *ctx)
{
    return error_pass_int(_art_fold(art, prefix, WITH_KEY, FORWARD, fold, ctx));
}

int art_fold_pr(art_ct art, str_const_ct prefix, art_fold_cb fold, void *ctx)
{
    return error_pass_int(_art_fold(art, prefix, WITHOUT_KEY, BACKWARD, fold, ctx));
}

int art_fold_prk(art_ct art, str_const_ct prefix, art_fold_cb fold, void *ctx)
{
    return error_pass_int(_art_fold(art, prefix, WITH_KEY, BACKWARD, fold, ctx));
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
        lookup = art_path_new(prefix);
        node = art_lookup(art, art->root, lookup, &node_prefix, NULL);
        return_error_if_fail(str_is_empty(lookup), E_ART_NOT_FOUND, NULL);
    }
    else
        node = art->root;
    
    if(!(path = str_dup_b(art_node_get_path(node)+node_prefix, node->path_len-node_prefix)))
        return error_wrap(), NULL;
    
    // nodes with 1 child may happen if a merge fails
    while(node->size == 1)
    {
        node = art_node_get_first_child(node);
        
        if(!str_append_set(path, 1, node->key)
        || !str_append_b(path, art_node_get_path(node), node->path_len))
            return error_wrap(), str_unref(path), NULL;
    }
    
    return path;
}
