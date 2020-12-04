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

/// \file

#ifndef YTIL_DEF_OS_H_INCLUDED
#define YTIL_DEF_OS_H_INCLUDED


#if DOXYGEN


#define OS_WINDOWS  1       ///< defined on windows
#define OS_MINGW    1       ///< defined on windows/mingw
#define OS_UNIX     1       ///< defined on unix
#define OS_LINUX    1       ///< defined on unix/linux
#define OS_ANDROID  1       ///< defined on unix/linux/android
#define OS_BSD      1       ///< defined on unix/bsd
#define OS_FREEBSD  1       ///< defined on unix/bsd/freebsd
#define OS_NETBSD   1       ///< defined on unix/bsd/netbsd
#define OS_OPENBSD  1       ///< defined on unix/bsd/openbsd
#define OS_SOLARIS  1       ///< defined on unix/solaris


#else // if DOXYGEN


#if defined(__MINGW32__) || defined(__MINGW64__)
    #define OS_MINGW 1
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || OS_MINGW
    #define OS_WINDOWS 1
#endif

#if defined(__ANDROID__)
    #define OS_ANDROID 1
#endif

#if defined(linux) || defined(__linux) || defined(__linux__) || OS_ANDROID
    #define OS_LINUX 1
#endif

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    #define OS_FREEBSD 1
#endif

#if defined(__NetBSD__)
    #define OS_NETBSD 1
#endif

#if defined(__OpenBSD__)
    #define OS_OPENBSD 1
#endif

#if defined(BSD) || OS_FREEBSD || OS_NETBSD || OS_OPENBSD
    #define OS_BSD 1
#endif

#if defined(sun) || defined(__sun)
    #define OS_SOLARIS 1
#endif

#if defined(unix) || defined(__unix) || defined(__unix__) || OS_LINUX || OS_BSD || OS_SOLARIS
    #define OS_UNIX 1
#endif


#endif // if DOXYGEN


/// Insert different values on unix and windows.
///
/// \param u    value to insert on unix
/// \param w    value to insert on windows
#if DOXYGEN
    #define UNIX_WINDOWS(u, w)
#elif OS_UNIX
    #define UNIX_WINDOWS(u, w) u
#elif OS_WINDOWS
    #define UNIX_WINDOWS(u, w) w
#endif

/// Insert value only on unix.
///
/// \param u    value to insert
#define UNIX(u) \
    UNIX_WINDOWS(u, )

/// Insert value only on windows.
///
/// \param w    value to insert
#define WINDOWS(w) \
    UNIX_WINDOWS(, w)

/// Insert object member only on unix.
///
/// \param m    member to insert
#define UNIX_MEMBER(m) \
    UNIX(m;)

/// Insert object member only on windows.
///
/// \param m    member to insert
#define WINDOWS_MEMBER(m) \
    WINDOWS(m;)


#endif // ifndef YTIL_DEF_OS_H_INCLUDED
