/*
 * Copyright (c) 2019 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_EXT_ENDIAN_H_INCLUDED
#define YTIL_EXT_ENDIAN_H_INCLUDED

#if defined(__linux__)

#   include <endian.h>

#elif defined(_WIN32)

#   include <sys/param.h>

#   if __BYTE_ORDER == __LITTLE_ENDIAN

#       define htole16(i) (i)
#       define htobe16(i) __builtin_bswap16(i)
#       define le16toh(i) (i)
#       define be16toh(i) __builtin_bswap16(i)
#       define htole32(i) (i)
#       define htobe32(i) __builtin_bswap32(i)
#       define le32toh(i) (i)
#       define be32toh(i) __builtin_bswap32(i)
#       define htole64(i) (i)
#       define htobe64(i) __builtin_bswap64(i)
#       define le64toh(i) (i)
#       define be64toh(i) __builtin_bswap64(i)

#   elif __BYTE_ORDER == __BIG_ENDIAN

#       define htole16(i) __builtin_bswap16(i)
#       define htobe16(i) (i)
#       define le16toh(i) __builtin_bswap16(i)
#       define be16toh(i) (i)
#       define htole32(i) __builtin_bswap32(i)
#       define htobe32(i) (i)
#       define le32toh(i) __builtin_bswap32(i)
#       define be32toh(i) (i)
#       define htole64(i) __builtin_bswap64(i)
#       define htobe64(i) (i)
#       define le64toh(i) __builtin_bswap64(i)
#       define be64toh(i) (i)

#   endif

#else

#   error unsupported platform

#endif

#endif
