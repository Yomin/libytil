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

#include <ytil/con/list.h>
#include <ytil/def.h>
#include <ytil/magic.h>

#include <stdlib.h>
#include <string.h>

#define LIST_FOREACH(list, field, node) \
    for(node = list->head.field; node != &list->head; node = node->field)

#define LIST_FOREACH_SAFE(list, field, node, next) \
    for(node = list->head.field, next = node->field; \
        node != &list->head; \
        node = next, next = node->field)


enum def
{
      MAGIC      = define_magic('L', 'S', 'T')
    , NODE_MAGIC = define_magic('L', 'N', 'O')
};

typedef struct list_node
{
    DEBUG_MAGIC
    DEBUG_MEMBER(list_ct list)
    
    list_node_ct prev, next;
    void *data;
} list_node_st;

typedef struct list
{
    DEBUG_MAGIC
    
    size_t size;
    list_node_st head;
} list_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_LIST_EMPTY, "List is empty.")
    , ERROR_INFO(E_LIST_END, "List end.")
    , ERROR_INFO(E_LIST_NOT_FOUND, "Node not found.")
    , ERROR_INFO(E_LIST_OUT_OF_BOUNDS, "Out of bounds access.")
};


list_ct list_new(void)
{
    list_ct list;
    
    if(!(list = calloc(1, sizeof(list_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic(list);
    list->head.next = list->head.prev = &list->head;
    
    return list;
}

void list_free(list_ct list)
{
    list_free_f(list, NULL, NULL);
}

void list_free_f(list_ct list, list_dtor_cb dtor, void *ctx)
{
    list_clear_f(list, dtor, ctx);
    
    free(list);
}

list_ct list_free_if_empty(list_ct list)
{
    assert_magic(list);
    
    if(list->size)
        return list;
    
    list_free_f(list, NULL, NULL);
    
    return NULL;
}

void list_clear(list_ct list)
{
    list_clear_f(list, NULL, NULL);
}

void list_clear_f(list_ct list, list_dtor_cb dtor, void *ctx)
{
    list_node_ct node, next;
    
    assert_magic(list);
    
    LIST_FOREACH_SAFE(list, next, node, next)
    {
        if(dtor)
            dtor(list, node->data, ctx);
        
        free(node);
    }
    
    list->size = 0;
    list->head.next = list->head.prev = &list->head;
}

static list_node_ct list_node_new(list_ct list, void *data)
{
    list_node_ct node;
    
    if(!(node = calloc(1, sizeof(list_node_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic_n(node, NODE_MAGIC);
    DEBUG(node->list = list);
    node->data = data;
    
    return node;
}

list_ct list_clone(list_const_ct list)
{
    return error_propagate_ptr(list_clone_f(list, NULL, NULL, NULL));
}

list_ct list_clone_f(list_const_ct list1, list_clone_cb clone, list_dtor_cb dtor, void *ctx)
{
    list_ct list2;
    list_node_ct node1, node2;
    void *data;
    
    assert_magic(list1);
    
    if(!(list2 = list_new()))
        return error_propagate(), NULL;
    
    node2 = &list2->head;
    
    LIST_FOREACH(list1, next, node1)
    {
        data = node1->data;
        
        if(clone && clone(list1, &data, node1->data, ctx))
        {
            node2->next = &list2->head;
            list_free_f(list2, dtor, ctx);
            
            return error_wrap(), NULL;
        }
        
        if(!(node2->next = list_node_new(list2, data)))
        {
            if(dtor)
                dtor(list2, data, ctx);
            
            node2->next = &list2->head;
            list_free_f(list2, dtor, ctx);
            
            return error_propagate(), NULL;
        }
        
        node2->next->prev = node2;
        node2 = node2->next;
    }
    
    node2->next = &list2->head;
    list2->head.prev = node2;
    list2->size = list1->size;
    
    return list2;
}

bool list_is_empty(list_const_ct list)
{
    assert_magic(list);
    
    return !list->size;
}

size_t list_size(list_const_ct list)
{
    assert_magic(list);
    
    return list->size;
}

size_t list_memsize(list_const_ct list)
{
    return list_memsize_f(list, NULL, NULL);
}

size_t list_memsize_f(list_const_ct list, list_size_cb sfun, void *ctx)
{
    list_node_ct node;
    size_t size;
    
    assert_magic(list);
    
    size = sizeof(list_st) + list->size * sizeof(list_node_st);
    
    if(sfun)
        LIST_FOREACH(list, next, node)
            size += sfun(list, node->data, ctx);
    
    return size;
}

void *list_node_get_data(list_node_const_ct node)
{
    assert_magic_n(node, NODE_MAGIC);
    
    return node->data;
}

void list_node_set_data(list_node_ct node, void *data)
{
    assert_magic_n(node, NODE_MAGIC);
    
    node->data = data;
}

static list_node_ct list_get_node(list_const_ct list, ssize_t pos)
{
    list_node_ct node;
    
    if(pos < 0)
        pos += list->size;
    
    return_error_if_fail(pos >= 0 && (size_t)pos < list->size, E_LIST_OUT_OF_BOUNDS, NULL);
    
    if((size_t)pos < list->size/2)
        for(node = list->head.next; pos; pos--, node = node->next);
    else
        for(node = list->head.prev, pos = list->size-pos-1; pos; pos--, node = node->prev);
    
    return node;
}

list_node_ct list_at(list_const_ct list, ssize_t pos)
{
    assert_magic(list);
    
    return error_propagate_ptr(list_get_node(list, pos));
}

list_node_ct list_first(list_const_ct list)
{
    assert_magic(list);
    return_error_if_fail(list->size, E_LIST_EMPTY, NULL);
    
    return list->head.next;
}

list_node_ct list_last(list_const_ct list)
{
    assert_magic(list);
    return_error_if_fail(list->size, E_LIST_EMPTY, NULL);
    
    return list->head.prev;
}

list_node_ct list_next(list_const_ct list, list_node_const_ct node)
{
    assert_magic(list);
    assert_magic_n(node, NODE_MAGIC);
    assert(list == node->list);
    return_error_if_fail(node->next != &list->head, E_LIST_END, NULL);
    
    return node->next;
}

list_node_ct list_prev(list_const_ct list, list_node_const_ct node)
{
    assert_magic(list);
    assert_magic_n(node, NODE_MAGIC);
    assert(list == node->list);
    return_error_if_fail(node->prev != &list->head, E_LIST_END, NULL);
    
    return node->prev;
}

void *list_data_at(list_const_ct list, ssize_t pos)
{
    list_node_ct node;
    
    assert_magic(list);
    
    if(!(node = list_get_node(list, pos)))
        return error_propagate(), NULL;
    
    return node->data;
}

void *list_data_first(list_const_ct list)
{
    assert_magic(list);
    return_error_if_fail(list->size, E_LIST_EMPTY, NULL);
    
    return list->head.next->data;
}

void *list_data_last(list_const_ct list)
{
    assert_magic(list);
    return_error_if_fail(list->size, E_LIST_EMPTY, NULL);
    
    return list->head.prev->data;
}

ssize_t list_pos(list_const_ct list, list_node_const_ct node)
{
    list_node_ct n;
    size_t pos;
    
    assert_magic(list);
    assert_magic_n(node, NODE_MAGIC);
    assert(list == node->list);
    
    for(pos = 0, n = list->head.next; n != &list->head; pos++, n = n->next)
        if(n == node)
            return pos;
    
    abort();
}

static list_node_ct _list_insert_after(list_ct list, list_node_const_ct cpre, void *data)
{
    list_node_ct node, pre = (list_node_ct)cpre;
    
    if(!(node = list_node_new(list, data)))
        return error_propagate(), NULL;
    
    node->prev = pre;
    node->next = pre->next;
    pre->next->prev = node;
    pre->next = node;
    
    list->size++;
    
    return node;
}

list_node_ct list_prepend(list_ct list, void *data)
{
    assert_magic(list);
    
    return error_propagate_ptr(_list_insert_after(list, &list->head, data));
}

list_node_ct list_append(list_ct list, void *data)
{
    assert_magic(list);
    
    return error_propagate_ptr(_list_insert_after(list, list->head.prev, data));
}

list_node_ct list_insert(list_ct list, ssize_t pos, void *data)
{
    list_node_ct node;
    
    assert_magic(list);
    
    if(ABS(pos) == list->size)
        node = pos < 0 ? list->head.next : &list->head;
    else if(!(node = list_get_node(list, pos)))
        return error_propagate(), NULL;
    
    return error_propagate_ptr(_list_insert_after(list, node->prev, data));
}

list_node_ct list_insert_before(list_ct list, list_node_const_ct suc, void *data)
{
    assert_magic(list);
    assert_magic_n(suc, NODE_MAGIC);
    assert(list == suc->list);
    
    return error_propagate_ptr(_list_insert_after(list, suc->prev, data));
}

list_node_ct list_insert_after(list_ct list, list_node_const_ct pre, void *data)
{
    assert_magic(list);
    assert_magic_n(pre, NODE_MAGIC);
    assert(list == pre->list);
    
    return error_propagate_ptr(_list_insert_after(list, pre, data));
}

static void _list_remove(list_ct list, list_node_ct node, list_dtor_cb dtor, void *ctx)
{
    if(dtor)
        dtor(list, node->data, ctx);
    
    node->next->prev = node->prev;
    node->prev->next = node->next;
    
    free(node);
    
    list->size--;
}

void list_remove(list_ct list, list_node_ct node)
{
    assert_magic(list);
    assert_magic_n(node, NODE_MAGIC);
    assert(list == node->list);
    
    _list_remove(list, node, NULL, NULL);
}

int list_remove_at(list_ct list, ssize_t pos)
{
    return error_propagate_int(list_remove_at_f(list, pos, NULL, NULL));
}

int list_remove_at_f(list_ct list, ssize_t pos, list_dtor_cb dtor, void *ctx)
{
    list_node_ct node;
    
    assert_magic(list);
    
    if(!(node = list_get_node(list, pos)))
        return error_propagate(), -1;
    
    _list_remove(list, node, dtor, ctx);
    
    return 0;
}

list_node_ct list_find(list_const_ct list, list_pred_cb pred, void *ctx)
{
    list_node_ct node;
    
    assert_magic(list);
    assert(pred);
    
    LIST_FOREACH(list, next, node)
        if(pred(list, node->data, ctx))
            return node;
    
    return_error_if_reached(E_LIST_NOT_FOUND, NULL);
}

list_node_ct list_find_r(list_const_ct list, list_pred_cb pred, void *ctx)
{
    list_node_ct node;
    
    assert_magic(list);
    assert(pred);
    
    LIST_FOREACH(list, prev, node)
        if(pred(list, node->data, ctx))
            return node;
    
    return_error_if_reached(E_LIST_NOT_FOUND, NULL);
}

int list_find_remove(list_ct list, list_pred_cb pred, void *ctx)
{
    return error_propagate_int(list_find_remove_f(list, pred, ctx, NULL, NULL));
}

int list_find_remove_f(list_ct list, list_pred_cb pred, void *pred_ctx, list_dtor_cb dtor, void *dtor_ctx)
{
    list_node_ct node, next;
    
    assert_magic(list);
    assert(pred);
    
    LIST_FOREACH_SAFE(list, next, node, next)
        if(pred(list, node->data, pred_ctx))
            return _list_remove(list, node, dtor, dtor_ctx), 0;
    
    return_error_if_reached(E_LIST_NOT_FOUND, -1);
}

int list_find_remove_r(list_ct list, list_pred_cb pred, void *ctx)
{
    return error_propagate_int(list_find_remove_rf(list, pred, ctx, NULL, NULL));
}

int list_find_remove_rf(list_ct list, list_pred_cb pred, void *pred_ctx, list_dtor_cb dtor, void *dtor_ctx)
{
    list_node_ct node, prev;
    
    assert_magic(list);
    assert(pred);
    
    LIST_FOREACH_SAFE(list, prev, node, prev)
        if(pred(list, node->data, pred_ctx))
            return _list_remove(list, node, dtor, dtor_ctx), 0;
    
    return_error_if_reached(E_LIST_NOT_FOUND, -1);
}

size_t list_find_remove_all(list_ct list, list_pred_cb pred, void *ctx)
{
    return list_find_remove_all_f(list, pred, ctx, NULL, NULL);
}

size_t list_find_remove_all_f(list_ct list, list_pred_cb pred, void *pred_ctx, list_dtor_cb dtor, void *dtor_ctx)
{
    list_node_ct node, next;
    size_t count = 0;
    
    assert_magic(list);
    assert(pred);
    
    LIST_FOREACH_SAFE(list, next, node, next)
        if(pred(list, node->data, pred_ctx))
        {
            _list_remove(list, node, dtor, dtor_ctx);
            count++;
        }
    
    return count;
}

void list_swap(list_node_const_ct cnode1, list_node_const_ct cnode2)
{
    list_node_ct node1 = (list_node_ct)cnode1, node2 = (list_node_ct)cnode2, tmp;
    
    assert_magic_n(node1, NODE_MAGIC);
    assert_magic_n(node2, NODE_MAGIC);
    assert(node1->list == node2->list);
    
    node1->prev->next = node2;
    node2->prev->next = node1;
    node1->next->prev = node2;
    node2->next->prev = node1;
    
    tmp = node1->prev;
    node1->prev = node2->prev;
    node2->prev = tmp;
    
    tmp = node1->next;
    node1->next = node2->next;
    node2->next = tmp;
}

int list_fold(list_const_ct list, list_fold_cb fold, void *ctx)
{
    list_node_ct node, next;
    int rc;
    
    assert_magic(list);
    assert(fold);
    
    LIST_FOREACH_SAFE(list, next, node, next)
        if((rc = error_wrap_int(fold(list, node->data, ctx))))
            return rc;
    
    return 0;
}

int list_fold_r(list_const_ct list, list_fold_cb fold, void *ctx)
{
    list_node_ct node, prev;
    int rc;
    
    assert_magic(list);
    assert(fold);
    
    LIST_FOREACH_SAFE(list, prev, node, prev)
        if((rc = error_wrap_int(fold(list, node->data, ctx))))
            return rc;
    
    return 0;
}
