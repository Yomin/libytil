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

#include <ytil/sys/reg.h>
#include <ytil/def.h>
#include <ytil/magic.h>

enum def
{
      MAGIC = define_magic('R', 'E', 'G')
};

typedef struct reg
{
    DEBUG_MAGIC
    
    int foo;
} reg_st;

/*static const error_info_st error_infos[] =
{
      ERROR_INFO(E_REG_INVALID_PATH, "Invalid registry path.")
};*/


reg_ct reg_open(path_const_ct path)
{
    reg_ct reg;
    
    if(!(reg = calloc(1, sizeof(reg_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic(reg);
    
    return reg;
}

void reg_close(reg_ct reg)
{
    assert_magic(reg);
    
    free(reg);
}

int reg_set_int(reg_const_ct reg, str_ct key, int value)
{
    assert_magic(reg);
    
    return 0;
}

int reg_set_str(reg_const_ct reg, str_ct key, str_ct value)
{
    assert_magic(reg);
    
    return 0;
}
