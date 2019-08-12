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

// format seconds into allocated human readable representation
char *time_strdup_duration(size_t seconds, size_t milli_seconds);

// set timespec with seconds
void time_ts_set_sec(timespec_st *ts, size_t seconds);
// set timespec with milli seconds
void time_ts_set_milli(timespec_st *ts, size_t millis);
// set timespec with micro seconds
void time_ts_set_micro(timespec_st *ts, size_t micros);
// set timespec with nano second
void time_ts_set_nano(timespec_st *ts, size_t nanos);

// add src timespec to dst timespec
void time_ts_add(timespec_st *dst, const timespec_st *src);
// add seconds to timespec
void time_ts_add_sec(timespec_st *ts, size_t seconds);
// add milli seconds to timespec
void time_ts_add_milli(timespec_st *ts, size_t millis);
// add micro seconds to timespec
void time_ts_add_micro(timespec_st *ts, size_t micros);
// add nano seconds to timespec
void time_ts_add_nano(timespec_st *ts, size_t nanos);

// return timespec in seconds
size_t time_ts_get_sec(const timespec_st *ts);
// return timespec in milli seconds
size_t time_ts_get_milli(const timespec_st *ts);
// return timespec in micro seconds
size_t time_ts_get_micro(const timespec_st *ts);
// return timespec in nano seconds
size_t time_ts_get_nano(const timespec_st *ts);

// return -1, 0, 1 if ts1 is less than, equal, greater than ts2
int time_ts_cmp(const timespec_st *ts1, const timespec_st *ts2);

// set dst timespec to end timespec - start timespec
void   time_ts_diff(timespec_st *dst, const timespec_st *start, const timespec_st *end);
// set dst timespec to end timeval - start timeval
void   time_ts_diff_tv(timeval_st *dst, const timespec_st *start, const timespec_st *end);
// return end timespec - start timespec in seconds
size_t time_ts_diff_sec(const timespec_st *start, const timespec_st *end);
// return end timespec - start timespec in milli seconds
size_t time_ts_diff_milli(const timespec_st *start, const timespec_st *end);
// return end timespec - start timespec in micro seconds
size_t time_ts_diff_micro(const timespec_st *start, const timespec_st *end);
// return end timespec - start timespec in nano seconds
size_t time_ts_diff_nano(const timespec_st *start, const timespec_st *end);

// set timeval with seconds
void time_tv_set_sec(timeval_st *tv, size_t seconds);
// set timeval with milli seconds
void time_tv_set_milli(timeval_st *tv, size_t millis);
// set timeval with micro seconds
void time_tv_set_micro(timeval_st *tv, size_t micros);
// set timeval with nano second
void time_tv_set_nano(timeval_st *tv, size_t nanos);

// add src timeval to dst timeval
void time_tv_add(timeval_st *dst, const timeval_st *src);
// add seconds to timeval
void time_tv_add_sec(timeval_st *tv, size_t seconds);
// add milli seconds to timeval
void time_tv_add_milli(timeval_st *tv, size_t millis);
// add micro seconds to timeval
void time_tv_add_micro(timeval_st *tv, size_t micros);
// add nano seconds to timeval
void time_tv_add_nano(timeval_st *tv, size_t nanos);

// return timeval in seconds
size_t time_tv_get_sec(const timeval_st *tv);
// return timeval in milli seconds
size_t time_tv_get_milli(const timeval_st *tv);
// return timeval in micro seconds
size_t time_tv_get_micro(const timeval_st *tv);
// return timeval in nano seconds
size_t time_tv_get_nano(const timeval_st *tv);

// return -1, 0, 1 if tv1 is less than, equal, greater than tv2
int time_tv_cmp(const timeval_st *tv1, const timeval_st *tv2);

// set dst timeval to end timeval - start timeval
void   time_tv_diff(timeval_st *dst, const timeval_st *start, const timeval_st *end);
// set dst timeval to end timespec - start timespec
void   time_tv_diff_ts(timespec_st *dst, const timeval_st *start, const timeval_st *end);
// return end timeval - start timeval in seconds
size_t time_tv_diff_sec(const timeval_st *start, const timeval_st *end);
// return end timeval - start timeval in milli seconds
size_t time_tv_diff_milli(const timeval_st *start, const timeval_st *end);
// return end timeval - start timeval in micro seconds
size_t time_tv_diff_micro(const timeval_st *start, const timeval_st *end);
// return end timeval - start timeval in nano seconds
size_t time_tv_diff_nano(const timeval_st *start, const timeval_st *end);

#endif
