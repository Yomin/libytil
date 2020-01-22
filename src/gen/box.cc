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

#include <ytil/gen/box.h>
#include <ytil/def.h>
#include <ytil/magic.h>
#include <stdlib.h>

enum def
{
      MAGIC = define_magic('B', 'O', 'X')
};

typedef struct box
{
    DEBUG_MAGIC
    
    type_id type;
    void *value;
} box_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_BOX_INVALID_TYPE, "Invalid box type.")
};


box_ct box_new(type_id type, void *value)
{
    box_ct box;
    
    return_error_if_fail(type_is_valid(type), E_BOX_INVALID_TYPE, NULL);
    
    if(!(box = calloc(1, sizeof(box_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic(box);
    
    box->type = type;
    box->value = value;
    
    return box;
}

void box_free(box_ct box)
{
    assert_magic(box);
    
    type_free(box->type, box->value);
    
    free(box);
}

void *box_get(box_ct box)
{
    assert_magic(box);
    
    return box->value;
}

void *box_unpack(box_ct box)
{
    void *value;
    
    assert_magic(box);
    
    value = box->value;
    free(box);
    
    return value;
}

type_id box_type(box_ct box)
{
    assert_magic(box);
    
    return box->type;
}

size_t box_size(box_ct box)
{
    assert_magic(box);
    
    return type_size(box->type);
}

bool box_check(box_ct box, type_cap_id cap)
{
    assert_magic(box);
    
    return type_check(box->type, cap);
}

str_ct box_show(box_ct box)
{
    assert_magic(box);
    
    return error_wrap_ptr(type_show(box->type, box->value));
}

int box_cmp(box_ct box1, box_ct box2)
{
    assert_magic(box1);
    assert_magic(box2);
    assert(box1->type == box2->type);
    
    return type_cmp(box1->type, box1->value, box2->value);
}

bool box_equal(box_ct box1, box_ct box2)
{
    assert_magic(box1);
    assert_magic(box2);
    assert(box1->type == box2->type);
    
    return type_equal(box1->type, box1->value, box2->value);
}
