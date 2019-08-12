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

#include <ytil/ext/getopt.h>
#include <ytil/ext/stdio.h>
#include <stdbool.h>

typedef union sieve_state
{
    struct short_sieve
    {
        const char *opts;
        getopt_short_cb cb;
    } s;
    struct long_sieve
    {
        const getopt_option_st *opts;
        getopt_long_cb cb;
    } l;
} sieve_un;


static int sieve(bool mode, int *argc, char *argv[], sieve_un *state, void *ctx)
{
    int opt, ind, count, orig_optind, orig_opterr, rc = 0;
    
    orig_optind = optind;
    orig_opterr = opterr;
    optind = 0; // force reinitialization for '-'
    opterr = 0;
    
    while((!mode && (opt = getopt(*argc, argv, state->s.opts)) != -1)
    ||    ( mode && (opt = getopt_long(*argc, argv, "-", state->l.opts, &ind)) != -1))
    {
        if(opt == '?' || opt == 1)
            continue;
        
        if((!mode && (rc = state->s.cb(opt, optarg, ctx)))
        || ( mode && (rc = state->l.cb(ind, opt, optarg, ctx))))
            break;
        
        count = optarg ? 2 : 1;
        optind -= count;
        
        for(ind=optind; ind < *argc-count; ind++)
            argv[ind] = argv[ind+count];
        
        *argc -= count;
    }
    
    // disable '-'
    optind = 0;
    getopt(*argc, argv, "");
    
    optind = orig_optind;
    opterr = orig_opterr;
    
    return rc;
}

int getopt_sieve(int *argc, char *argv[], const char *opts, getopt_short_cb cb, void *ctx)
{
    sieve_un state = { .s.opts = FMT("-%s", opts), .s.cb = cb };
    
    return sieve(false, argc, argv, &state, ctx);
}

int getopt_long_sieve(int *argc, char *argv[], const getopt_option_st *opts, getopt_long_cb cb, void *ctx)
{
    sieve_un state = { .l.opts = opts, .l.cb = cb };
    
    return sieve(true, argc, argv, &state, ctx);
}
