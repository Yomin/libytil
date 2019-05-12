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

#ifndef __YTIL_EXT_TIME_H__
#define __YTIL_EXT_TIME_H__

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

typedef struct tm       tm_st;
typedef struct timespec timespec_st;
typedef struct timeval  timeval_st;


// format date into static buffer
char *time_isodate(const tm_st *tm);
// format time into static buffer
char *time_isotime(const tm_st *tm);
// format date and time into static buffer
char *time_isots(const tm_st *tm);

// format date into user supplied buffer (min 11 bytes)
char *time_isodate_r(const tm_st *tm, char *buf);
// format time into user supplied buffer (min 9 bytes)
char *time_isotime_r(const tm_st *tm, char *buf);
// format date and time into user supplied buffer (min 20 bytes)
char *time_isots_r(const tm_st *tm, char *buf);

// format date into allocated buffer
char *time_strdup_isodate(const tm_st *tm);
// format time into allocated buffer
char *time_strdup_isotime(const tm_st *tm);
// format date and time into allocated buffer
char *time_strdup_isots(const tm_st *tm);

// format interval into allocated human readable representation
char *time_strdup_duration(time_t from, time_t till);

// add src timespec to dst timespec
void time_ts_add(timespec_st *dst, const timespec_st *src);
// add seconds and milliseconds to timespec
void time_ts_add_s(timespec_st *dst, size_t sec, size_t msec);
// return -1, 0, 1 if tsp1 is less than, equal, greater than tsp2
int  time_ts_cmp(const timespec_st *tsp1, const timespec_st *tsp2);
// set dst timespec to end timespec - start timespec
void time_ts_diff(timespec_st *dst, const timespec_st *start, const timespec_st *end);
// set dst timespec to end timeval - start timeval
void time_ts_diff_tv(timespec_st *dst, const timeval_st *start, const timeval_st *end);

// add src timeval to dst timeval
void time_tv_add(timeval_st *dst, const timeval_st *src);
// add seconds and milliseconds to timeval
void time_tv_add_s(timeval_st *dst, size_t sec, size_t msec);
// return -1, 0, 1 if tsp1 is less than, equal, greater than tsp2
int  time_tv_cmp(const timeval_st *tv1, const timeval_st *tv2);
// set dst timeval to end timeval - start timeval
void time_tv_diff(timeval_st *dst, const timeval_st *start, const timeval_st *end);
// set dst timeval to end timespec - start timespec
void time_tv_diff_ts(timeval_st *dst, const timespec_st *start, const timespec_st *end);

#endif
