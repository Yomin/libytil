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

#include "time.h"
#include "stdio.h"
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

char *time_strdup_duration(time_t from, time_t till)
{
    int years, days, hours, mins, secs;
    time_t diff;
    
    assert(till >> from);
    
    diff  = till - from;
    secs  = diff % 60;  diff /= 60;
    mins  = diff % 60;  diff /= 60;
    hours = diff % 24;  diff /= 24;
    days  = diff % 365; diff /= 365;
    years = diff;
    
    if(years)
        return strdup_printf("%iy %id %02i:%02i:%02i", years, days, hours, mins, secs);
    if(days)
        return strdup_printf("%id %02i:%02i:%02i", days, hours, mins, secs);
    if(hours)
        return strdup_printf("%02i:%02i:%02i", hours, mins, secs);
    if(mins)
        return strdup_printf("%02i:%02i", mins, secs);
    
    return strdup_printf("%i seconds", secs);
}

void time_ts_add(timespec_st *dst, const timespec_st *src)
{
    assert(src && src->tv_nsec < 1000000000);
    assert(dst && dst->tv_nsec < 1000000000);
    
    dst->tv_sec += src->tv_sec;
    dst->tv_sec += (dst->tv_nsec + src->tv_nsec) / 1000000000;
    dst->tv_nsec = (dst->tv_nsec + src->tv_nsec) % 1000000000;
}

void time_ts_add_s(timespec_st *dst, size_t sec, size_t msec)
{
    assert(dst && dst->tv_nsec < 1000000000);
    
    sec += msec/1000;
    msec %= 1000;
    
    dst->tv_sec += sec;
    dst->tv_sec += (dst->tv_nsec + msec*1000000) / 1000000000;
    dst->tv_nsec = (dst->tv_nsec + msec*1000000) % 1000000000;
}

int time_ts_cmp(const timespec_st *tsp1, const timespec_st *tsp2)
{
    assert(tsp1 && tsp1->tv_nsec < 1000000000);
    assert(tsp2 && tsp2->tv_nsec < 1000000000);
    
    if(tsp1->tv_sec < tsp2->tv_sec)
        return -1;
    
    if(tsp1->tv_sec > tsp2->tv_sec)
        return 1;
    
    if(tsp1->tv_nsec < tsp2->tv_nsec)
        return -1;
    
    if(tsp1->tv_nsec > tsp2->tv_nsec)
        return 1;
    
    return 0;
}

void time_ts_diff(timespec_st *dst, const timespec_st *start, const timespec_st *end)
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

void time_ts_diff_tv(timespec_st *dst, const timeval_st *start, const timeval_st *end)
{
    assert(time_tv_cmp(start, end) <= 0);
    
    dst->tv_sec = end->tv_sec - start->tv_sec;
    
    if(start->tv_usec <= end->tv_usec)
        dst->tv_nsec = (end->tv_usec - start->tv_usec) * 1000;
    else
    {
        dst->tv_sec--;
        dst->tv_nsec = (1000000 - start->tv_usec + end->tv_usec) * 1000;
    }
}

void time_tv_add(timeval_st *dst, const timeval_st *src)
{
    assert(src && src->tv_usec < 1000000);
    assert(dst && dst->tv_usec < 1000000);
    
    dst->tv_sec += src->tv_sec;
    dst->tv_sec += (dst->tv_usec + src->tv_usec) / 1000000;
    dst->tv_usec = (dst->tv_usec + src->tv_usec) % 1000000;
}

void time_tv_add_s(timeval_st *dst, size_t sec, size_t msec)
{
    assert(dst && dst->tv_usec < 1000000);
    
    sec += msec/1000;
    msec %= 1000;
    
    dst->tv_sec += sec;
    dst->tv_sec += (dst->tv_usec + msec*1000) / 1000000;
    dst->tv_usec = (dst->tv_usec + msec*1000) % 1000000;
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

void time_tv_diff(timeval_st *dst, const timeval_st *start, const timeval_st *end)
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

void time_tv_diff_ts(timeval_st *dst, const timespec_st *start, const timespec_st *end)
{
    assert(time_ts_cmp(start, end) <= 0);
    
    dst->tv_sec = end->tv_sec - start->tv_sec;
    
    if(start->tv_nsec <= end->tv_nsec)
        dst->tv_usec = (end->tv_nsec - start->tv_nsec) / 1000;
    else
    {
        dst->tv_sec--;
        dst->tv_usec = (1000000000 - start->tv_nsec + end->tv_nsec) / 1000;
    }
}
