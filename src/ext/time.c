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

#include <ytil/ext/time.h>
#include <ytil/ext/stdio.h>
#include <assert.h>
#include <stdlib.h>


char *time_isodate(const tm_st *tm)
{
    static char buf[11];
    
    return time_isodate_r(tm, buf);
}

char *time_isotime(const tm_st *tm)
{
    static char buf[9];
    
    return time_isotime_r(tm, buf);
}

char *time_isots(const tm_st *tm)
{
    static char buf[20];
    
    return time_isots_r(tm, buf);
}

char *time_isodate_r(const tm_st *tm, char *buf)
{
    snprintf(buf, 11, "%04u-%02u-%02u", tm->tm_year + 1900, tm->tm_mon +1, tm->tm_mday);
    
    return buf;
}

char *time_isotime_r(const tm_st *tm, char *buf)
{
    snprintf(buf, 9, "%02u:%02u:%02u", tm->tm_hour, tm->tm_min, tm->tm_sec);
    
    return buf;
}

char *time_isots_r(const tm_st *tm, char *buf)
{
    snprintf(buf, 20, "%04u-%02u-%02u %02u:%02u:%02u",
        tm->tm_year + 1900, tm->tm_mon +1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
    
    return buf;
}

char *time_strdup_isodate(const tm_st *tm)
{
    char *buf;
    
    if(!(buf = malloc(11)))
        return NULL;
    
    return time_isodate_r(tm, buf);
}

char *time_strdup_isotime(const tm_st *tm)
{
    char *buf;
    
    if(!(buf = malloc(9)))
        return NULL;
    
    return time_isotime_r(tm, buf);
}

char *time_strdup_isots(const tm_st *tm)
{
    char *buf;
    
    if(!(buf = malloc(20)))
        return NULL;
    
    return time_isots_r(tm, buf);
}

char *time_strdup_duration(size_t seconds, size_t milli_seconds)
{
    unsigned int years, days, hours, mins, secs, msecs;
    
    seconds += milli_seconds / 1000;
    
    msecs = milli_seconds % 1000;
    secs  = seconds % 60;  seconds /= 60;
    mins  = seconds % 60;  seconds /= 60;
    hours = seconds % 24;  seconds /= 24;
    days  = seconds % 365; seconds /= 365;
    years = seconds;
    
    if(years)
        return strdup_printf("%uy %ud %02u:%02u:%02u", years, days, hours, mins, secs);
    if(days)
        return strdup_printf("%ud %02u:%02u:%02u", days, hours, mins, secs);
    if(hours)
        return strdup_printf("%02u:%02u:%02u", hours, mins, secs);
    if(mins)
        return strdup_printf("%02u:%02u", mins, secs);
    
    return strdup_printf("%u.%03u", secs, msecs);
}

void time_ts_set_diff(timespec_st *dst, const timespec_st *start, const timespec_st *end)
{
    assert(time_ts_cmp(start, end) <= 0);
    
    dst->tv_sec = end->tv_sec - start->tv_sec;
    
    if(start->tv_nsec <= end->tv_nsec)
        dst->tv_nsec = end->tv_nsec - start->tv_nsec;
    else
    {
        dst->tv_sec--;
        dst->tv_nsec = 1000000000 - start->tv_nsec + end->tv_nsec;
    }
}

void time_ts_set_diff_tv(timespec_st *dst, const timeval_st *start, const timeval_st *end)
{
    timeval_st tv;
    
    assert(dst);
    
    time_tv_set_diff(&tv, start, end);
    
    dst->tv_sec = tv.tv_sec;
    dst->tv_nsec = tv.tv_usec * 1000;
}

void time_ts_set_sec(timespec_st *ts, size_t seconds)
{
    assert(ts);
    
    ts->tv_sec = seconds;
    ts->tv_nsec = 0;
}

void time_ts_set_milli(timespec_st *ts, size_t millis)
{
    assert(ts);
    
    ts->tv_sec = millis / 1000;
    ts->tv_nsec = (millis % 1000) * 1000000;
}

void time_ts_set_micro(timespec_st *ts, size_t micros)
{
    assert(ts);
    
    ts->tv_sec = micros / 1000000;
    ts->tv_nsec = (micros % 1000000) * 1000;
}

void time_ts_set_nano(timespec_st *ts, size_t nanos)
{
    assert(ts);
    
    ts->tv_sec = nanos / 1000000000;
    ts->tv_nsec = nanos % 1000000000;
}

void time_ts_add(timespec_st *dst, const timespec_st *src)
{
    assert(src && src->tv_nsec < 1000000000);
    assert(dst && dst->tv_nsec < 1000000000);
    
    dst->tv_sec += src->tv_sec;
    dst->tv_sec += (dst->tv_nsec + src->tv_nsec) / 1000000000;
    dst->tv_nsec = (dst->tv_nsec + src->tv_nsec) % 1000000000;
}

void time_ts_add_tv(timespec_st *dst, const timeval_st *src)
{
    assert(src && src->tv_usec < 1000000);
    assert(dst && dst->tv_nsec < 1000000000);
    
    dst->tv_sec += src->tv_sec;
    dst->tv_sec += (dst->tv_nsec + src->tv_usec*1000) / 1000000000;
    dst->tv_nsec = (dst->tv_nsec + src->tv_usec*1000) % 1000000000;
}

void time_ts_add_diff(timespec_st *dst, const timespec_st *start, const timespec_st *end)
{
    timespec_st ts;
    
    time_ts_set_diff(&ts, start, end);
    time_ts_add(dst, &ts);
}

void time_ts_add_diff_tv(timespec_st *dst, const timeval_st *start, const timeval_st *end)
{
    timeval_st tv;
    
    time_tv_set_diff(&tv, start, end);
    time_ts_add_tv(dst, &tv);
}

void time_ts_add_sec(timespec_st *ts, size_t sec)
{
    assert(ts);
    
    ts->tv_sec += sec;
}

void time_ts_add_milli(timespec_st *ts, size_t millis)
{
    assert(ts && ts->tv_nsec < 1000000000);
    
    ts->tv_sec += millis / 1000;
    millis = millis % 1000;
    ts->tv_sec += (ts->tv_nsec + millis*1000000) / 1000000000;
    ts->tv_nsec = (ts->tv_nsec + millis*1000000) % 1000000000;
}

void time_ts_add_micro(timespec_st *ts, size_t micros)
{
    assert(ts && ts->tv_nsec < 1000000000);
    
    ts->tv_sec += micros / 1000000;
    micros = micros % 1000000;
    ts->tv_sec += (ts->tv_nsec + micros*1000) / 1000000000;
    ts->tv_nsec = (ts->tv_nsec + micros*1000) % 1000000000;
}

void time_ts_add_nano(timespec_st *ts, size_t nanos)
{
    assert(ts && ts->tv_nsec < 1000000000);
    
    ts->tv_sec += nanos / 1000000000;
    nanos = nanos % 1000000000;
    ts->tv_sec += (ts->tv_nsec + nanos) / 1000000000;
    ts->tv_nsec = (ts->tv_nsec + nanos) % 1000000000;
}

size_t time_ts_get_sec(const timespec_st *ts)
{
    assert(ts);
    
    return ts->tv_sec;
}

size_t time_ts_get_milli(const timespec_st *ts)
{
    assert(ts);
    
    return ts->tv_sec * 1000 + ts->tv_nsec / 1000000;
}

size_t time_ts_get_micro(const timespec_st *ts)
{
    assert(ts);
    
    return ts->tv_sec * 1000000 + ts->tv_nsec / 1000;
}

size_t time_ts_get_nano(const timespec_st *ts)
{
    assert(ts);
    
    return ts->tv_sec * 1000000000 + ts->tv_nsec;
}

int time_ts_cmp(const timespec_st *ts1, const timespec_st *ts2)
{
    assert(ts1 && ts1->tv_nsec < 1000000000);
    assert(ts2 && ts2->tv_nsec < 1000000000);
    
    if(ts1->tv_sec < ts2->tv_sec)
        return -1;
    
    if(ts1->tv_sec > ts2->tv_sec)
        return 1;
    
    if(ts1->tv_nsec < ts2->tv_nsec)
        return -1;
    
    if(ts1->tv_nsec > ts2->tv_nsec)
        return 1;
    
    return 0;
}

size_t time_ts_diff_sec(const timespec_st *start, const timespec_st *end)
{
    timespec_st ts;
    
    time_ts_set_diff(&ts, start, end);
    
    return ts.tv_sec;
}

size_t time_ts_diff_milli(const timespec_st *start, const timespec_st *end)
{
    timespec_st ts;
    
    time_ts_set_diff(&ts, start, end);
    
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

size_t time_ts_diff_micro(const timespec_st *start, const timespec_st *end)
{
    timespec_st ts;
    
    time_ts_set_diff(&ts, start, end);
    
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

size_t time_ts_diff_nano(const timespec_st *start, const timespec_st *end)
{
    timespec_st ts;
    
    time_ts_set_diff(&ts, start, end);
    
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

void time_tv_set_diff(timeval_st *dst, const timeval_st *start, const timeval_st *end)
{
    assert(time_tv_cmp(start, end) <= 0);
    
    dst->tv_sec = end->tv_sec - start->tv_sec;
    
    if(start->tv_usec <= end->tv_usec)
        dst->tv_usec = end->tv_usec - start->tv_usec;
    else
    {
        dst->tv_sec--;
        dst->tv_usec = 1000000 - start->tv_usec + end->tv_usec;
    }
}

void time_tv_set_diff_ts(timeval_st *dst, const timespec_st *start, const timespec_st *end)
{
    timespec_st ts;
    
    assert(dst);
    
    time_ts_set_diff(&ts, start, end);
    
    dst->tv_sec = ts.tv_sec;
    dst->tv_usec = ts.tv_nsec / 1000;
}

void time_tv_set_sec(timeval_st *tv, size_t seconds)
{
    assert(tv);
    
    tv->tv_sec = seconds;
    tv->tv_usec = 0;
}

void time_tv_set_milli(timeval_st *tv, size_t millis)
{
    assert(tv);
    
    tv->tv_sec = millis / 1000;
    tv->tv_usec = (millis % 1000) * 1000;
}

void time_tv_set_micro(timeval_st *tv, size_t micros)
{
    assert(tv);
    
    tv->tv_sec = micros / 1000000;
    tv->tv_usec = micros % 1000000;
}

void time_tv_set_nano(timeval_st *tv, size_t nanos)
{
    assert(tv);
    
    tv->tv_sec = nanos / 1000000000;
    tv->tv_usec = (nanos % 1000000000) / 1000;
}

void time_tv_add(timeval_st *dst, const timeval_st *src)
{
    assert(src && src->tv_usec < 1000000);
    assert(dst && dst->tv_usec < 1000000);
    
    dst->tv_sec += src->tv_sec;
    dst->tv_sec += (dst->tv_usec + src->tv_usec) / 1000000;
    dst->tv_usec = (dst->tv_usec + src->tv_usec) % 1000000;
}

void time_tv_add_ts(timeval_st *dst, const timespec_st *src)
{
    assert(src && src->tv_nsec < 1000000000);
    assert(dst && dst->tv_usec < 1000000);
    
    dst->tv_sec += src->tv_sec;
    dst->tv_sec += (dst->tv_usec + src->tv_nsec/1000) / 1000000;
    dst->tv_usec = (dst->tv_usec + src->tv_nsec/1000) % 1000000;
}

void time_tv_add_diff(timeval_st *dst, const timeval_st *start, const timeval_st *end)
{
    timeval_st tv;
    
    time_tv_set_diff(&tv, start, end);
    time_tv_add(dst, &tv);
}

void time_tv_add_diff_ts(timeval_st *dst, const timespec_st *start, const timespec_st *end)
{
    timespec_st ts;
    
    time_ts_set_diff(&ts, start, end);
    time_tv_add_ts(dst, &ts);
}

void time_tv_add_sec(timeval_st *tv, size_t sec)
{
    assert(tv);
    
    tv->tv_sec += sec;
}

void time_tv_add_milli(timeval_st *tv, size_t millis)
{
    assert(tv && tv->tv_usec < 1000000);
    
    tv->tv_sec += millis / 1000;
    millis = millis % 1000;
    tv->tv_sec += (tv->tv_usec + millis*1000) / 1000000;
    tv->tv_usec = (tv->tv_usec + millis*1000) % 1000000;
}

void time_tv_add_micro(timeval_st *tv, size_t micros)
{
    assert(tv && tv->tv_usec < 1000000);
    
    tv->tv_sec += micros / 1000000;
    micros = micros % 1000000;
    tv->tv_sec += (tv->tv_usec + micros) / 1000000;
    tv->tv_usec = (tv->tv_usec + micros) % 1000000;
}

void time_tv_add_nano(timeval_st *tv, size_t nanos)
{
    assert(tv && tv->tv_usec < 1000000);
    
    tv->tv_sec += nanos / 1000000000;
    nanos = nanos % 1000000000;
    tv->tv_sec += (tv->tv_usec + nanos/1000) / 1000000;
    tv->tv_usec = (tv->tv_usec + nanos/1000) % 1000000;
}

size_t time_tv_get_sec(const timeval_st *tv)
{
    assert(tv);
    
    return tv->tv_sec;
}

size_t time_tv_get_milli(const timeval_st *tv)
{
    assert(tv);
    
    return tv->tv_sec * 1000 + tv->tv_usec / 1000;
}

size_t time_tv_get_micro(const timeval_st *tv)
{
    assert(tv);
    
    return tv->tv_sec * 1000000 + tv->tv_usec;
}

size_t time_tv_get_nano(const timeval_st *tv)
{
    assert(tv);
    
    return tv->tv_sec * 1000000000 + tv->tv_usec * 1000;
}

int time_tv_cmp(const timeval_st *tv1, const timeval_st *tv2)
{
    assert(tv1 && tv1->tv_usec < 1000000);
    assert(tv2 && tv2->tv_usec < 1000000);
    
    if(tv1->tv_sec < tv2->tv_sec)
        return -1;
    
    if(tv1->tv_sec > tv2->tv_sec)
        return 1;
    
    if(tv1->tv_usec < tv2->tv_usec)
        return -1;
    
    if(tv1->tv_usec > tv2->tv_usec)
        return 1;
    
    return 0;
}

size_t time_tv_diff_sec(const timeval_st *start, const timeval_st *end)
{
    timeval_st tv;
    
    time_tv_set_diff(&tv, start, end);
    
    return tv.tv_sec;
}

size_t time_tv_diff_milli(const timeval_st *start, const timeval_st *end)
{
    timeval_st tv;
    
    time_tv_set_diff(&tv, start, end);
    
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

size_t time_tv_diff_micro(const timeval_st *start, const timeval_st *end)
{
    timeval_st tv;
    
    time_tv_set_diff(&tv, start, end);
    
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

size_t time_tv_diff_nano(const timeval_st *start, const timeval_st *end)
{
    timeval_st tv;
    
    time_tv_set_diff(&tv, start, end);
    
    return tv.tv_sec * 1000000000 + tv.tv_usec * 1000;
}
