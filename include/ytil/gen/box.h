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

#ifndef __YTIL_GEN_BOX_H__
#define __YTIL_GEN_BOX_H__

#include <ytil/type/type.h>
#include <ytil/cast.h>

typedef enum box_error
{
      E_BOX_INVALID_TYPE
} box_error_id;

struct box;
typedef struct box       *box_ct;
typedef const struct box *box_const_ct;


// create new box of type
box_ct  box_new(type_id type, void *value);
#define box_new_value(type, value) \
        box_new(type, VALUE_TO_POINTER(value))
// free box and boxed value
void    box_free(box_ct box);

// get boxed value
void   *box_get(box_ct box);
#define box_get_value(box, type) \
        POINTER_TO_VALUE(box_get(box), type)
// get boxed value and free box
void   *box_unpack(box_ct box);
#define box_unpack_value(box, type) \
        POINTER_TO_VALUE(box_unpack(box), type)

// get box type
type_id box_type(box_ct box);
// get box size
size_t  box_size(box_ct box);
// check if box type has capability
bool    box_check(box_ct box, type_cap_id cap);
// show boxed value
str_ct  box_show(box_ct box);
// compare boxed values
int     box_cmp(box_ct box1, box_ct box2);
// equal boxed values
bool    box_equal(box_ct box1, box_ct box2);


#endif
