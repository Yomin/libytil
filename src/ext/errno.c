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

#include <ytil/ext/errno.h>
#include <stddef.h>

#define CASE_STRERRNO(err) case err: return #err;

const char *strerrno(int err)
{
    switch(err)
    {
#ifdef E2BIG
    CASE_STRERRNO(E2BIG)
#endif
#ifdef EACCES
    CASE_STRERRNO(EACCES)
#endif
#ifdef EADDRINUSE
    CASE_STRERRNO(EADDRINUSE)
#endif
#ifdef EADDRNOTAVAIL
    CASE_STRERRNO(EADDRNOTAVAIL)
#endif
#ifdef EAFNOSUPPORT
    CASE_STRERRNO(EAFNOSUPPORT)
#endif
#ifdef EAGAIN
    CASE_STRERRNO(EAGAIN)
#endif
#ifdef EALREADY
    CASE_STRERRNO(EALREADY)
#endif
#ifdef EBADE
    CASE_STRERRNO(EBADE)
#endif
#ifdef EBADFD
    CASE_STRERRNO(EBADFD)
#endif
#ifdef EBADMSG
    CASE_STRERRNO(EBADMSG)
#endif
#ifdef EBADR
    CASE_STRERRNO(EBADR)
#endif
#ifdef EBADRQC
    CASE_STRERRNO(EBADRQC)
#endif
#ifdef EBADSLT
    CASE_STRERRNO(EBADSLT)
#endif
#ifdef EBUSY
    CASE_STRERRNO(EBUSY)
#endif
#ifdef ECANCELED
    CASE_STRERRNO(ECANCELED)
#endif
#ifdef ECHILD
    CASE_STRERRNO(ECHILD)
#endif
#ifdef ECHRNG
    CASE_STRERRNO(ECHRNG)
#endif
#ifdef ECOMM
    CASE_STRERRNO(ECOMM)
#endif
#ifdef ECONNABORTED
    CASE_STRERRNO(ECONNABORTED)
#endif
#ifdef ECONNREFUSED
    CASE_STRERRNO(ECONNREFUSED)
#endif
#ifdef ECONNRESET
    CASE_STRERRNO(ECONNRESET)
#endif
#ifdef EDEADLK
    CASE_STRERRNO(EDEADLK)
#endif
#ifdef EDESTADDRREQ
    CASE_STRERRNO(EDESTADDRREQ)
#endif
#ifdef EDOM
    CASE_STRERRNO(EDOM)
#endif
#ifdef EDQUOT
    CASE_STRERRNO(EDQUOT)
#endif
#ifdef EEXIST
    CASE_STRERRNO(EEXIST)
#endif
#ifdef EFAULT
    CASE_STRERRNO(EFAULT)
#endif
#ifdef EFBIG
    CASE_STRERRNO(EFBIG)
#endif
#ifdef EHOSTDOWN
    CASE_STRERRNO(EHOSTDOWN)
#endif
#ifdef EHOSTUNREACH
    CASE_STRERRNO(EHOSTUNREACH)
#endif
#ifdef EHWPOISON
    CASE_STRERRNO(EHWPOISON)
#endif
#ifdef EIDRM
    CASE_STRERRNO(EIDRM)
#endif
#ifdef EILSEQ
    CASE_STRERRNO(EILSEQ)
#endif
#ifdef EINPROGRESS
    CASE_STRERRNO(EINPROGRESS)
#endif
#ifdef EINTR
    CASE_STRERRNO(EINTR)
#endif
#ifdef EINVAL
    CASE_STRERRNO(EINVAL)
#endif
#ifdef EIO
    CASE_STRERRNO(EIO)
#endif
#ifdef EISCONN
    CASE_STRERRNO(EISCONN)
#endif
#ifdef EISDIR
    CASE_STRERRNO(EISDIR)
#endif
#ifdef EISNAM
    CASE_STRERRNO(EISNAM)
#endif
#ifdef EKEYEXPIRED
    CASE_STRERRNO(EKEYEXPIRED)
#endif
#ifdef EKEYREJECTED
    CASE_STRERRNO(EKEYREJECTED)
#endif
#ifdef EKEYREVOKED
    CASE_STRERRNO(EKEYREVOKED)
#endif
#ifdef EL2HLT
    CASE_STRERRNO(EL2HLT)
#endif
#ifdef EL2NSYNC
    CASE_STRERRNO(EL2NSYNC)
#endif
#ifdef EL3HLT
    CASE_STRERRNO(EL3HLT)
#endif
#ifdef EL3RST
    CASE_STRERRNO(EL3RST)
#endif
#ifdef ELIBACC
    CASE_STRERRNO(ELIBACC)
#endif
#ifdef ELIBBAD
    CASE_STRERRNO(ELIBBAD)
#endif
#ifdef ELIBMAX
    CASE_STRERRNO(ELIBMAX)
#endif
#ifdef ELIBSCN
    CASE_STRERRNO(ELIBSCN)
#endif
#ifdef ELIBEXEC
    CASE_STRERRNO(ELIBEXEC)
#endif
#ifdef ELNRNG
    CASE_STRERRNO(ELNRNG)
#endif
#ifdef ELOOP
    CASE_STRERRNO(ELOOP)
#endif
#ifdef EMEDIUMTYPE
    CASE_STRERRNO(EMEDIUMTYPE)
#endif
#ifdef EMFILE
    CASE_STRERRNO(EMFILE)
#endif
#ifdef EMLINK
    CASE_STRERRNO(EMLINK)
#endif
#ifdef EMSGSIZE
    CASE_STRERRNO(EMSGSIZE)
#endif
#ifdef EMULTIHOP
    CASE_STRERRNO(EMULTIHOP)
#endif
#ifdef ENAMETOOLONG
    CASE_STRERRNO(ENAMETOOLONG)
#endif
#ifdef ENETDOWN
    CASE_STRERRNO(ENETDOWN)
#endif
#ifdef ENETRESET
    CASE_STRERRNO(ENETRESET)
#endif
#ifdef ENETUNREACH
    CASE_STRERRNO(ENETUNREACH)
#endif
#ifdef ENFILE
    CASE_STRERRNO(ENFILE)
#endif
#ifdef ENOANO
    CASE_STRERRNO(ENOANO)
#endif
#ifdef ENOBUFS
    CASE_STRERRNO(ENOBUFS)
#endif
#ifdef ENODATA
    CASE_STRERRNO(ENODATA)
#endif
#ifdef ENODEV
    CASE_STRERRNO(ENODEV)
#endif
#ifdef ENOENT
    CASE_STRERRNO(ENOENT)
#endif
#ifdef ENOEXEC
    CASE_STRERRNO(ENOEXEC)
#endif
#ifdef ENOKEY
    CASE_STRERRNO(ENOKEY)
#endif
#ifdef ENOLCK
    CASE_STRERRNO(ENOLCK)
#endif
#ifdef ENOLINK
    CASE_STRERRNO(ENOLINK)
#endif
#ifdef ENOMEDIUM
    CASE_STRERRNO(ENOMEDIUM)
#endif
#ifdef ENOMEM
    CASE_STRERRNO(ENOMEM)
#endif
#ifdef ENOMSG
    CASE_STRERRNO(ENOMSG)
#endif
#ifdef ENONET
    CASE_STRERRNO(ENONET)
#endif
#ifdef ENOPKG
    CASE_STRERRNO(ENOPKG)
#endif
#ifdef ENOPROTOOPT
    CASE_STRERRNO(ENOPROTOOPT)
#endif
#ifdef ENOSPC
    CASE_STRERRNO(ENOSPC)
#endif
#ifdef ENOSR
    CASE_STRERRNO(ENOSR)
#endif
#ifdef ENOSTR
    CASE_STRERRNO(ENOSTR)
#endif
#ifdef ENOSYS
    CASE_STRERRNO(ENOSYS)
#endif
#ifdef ENOTBLK
    CASE_STRERRNO(ENOTBLK)
#endif
#ifdef ENOTCONN
    CASE_STRERRNO(ENOTCONN)
#endif
#ifdef ENOTDIR
    CASE_STRERRNO(ENOTDIR)
#endif
#ifdef ENOTEMPTY
    CASE_STRERRNO(ENOTEMPTY)
#endif
#ifdef ENOTRECOVERABLE
    CASE_STRERRNO(ENOTRECOVERABLE)
#endif
#ifdef ENOTSOCK
    CASE_STRERRNO(ENOTSOCK)
#endif
#ifdef ENOTSUP
    CASE_STRERRNO(ENOTSUP)
#endif
#ifdef ENOTTY
    CASE_STRERRNO(ENOTTY)
#endif
#ifdef ENOTUNIQ
    CASE_STRERRNO(ENOTUNIQ)
#endif
#ifdef ENXIO
    CASE_STRERRNO(ENXIO)
#endif
#if defined(EOPNOTSUPP) && EOPNOTSUPP != ENOTSUP
    CASE_STRERRNO(EOPNOTSUPP)
#endif
#ifdef EOVERFLOW
    CASE_STRERRNO(EOVERFLOW)
#endif
#ifdef EOWNERDEAD
    CASE_STRERRNO(EOWNERDEAD)
#endif
#ifdef EPERM
    CASE_STRERRNO(EPERM)
#endif
#ifdef EPFNOSUPPORT
    CASE_STRERRNO(EPFNOSUPPORT)
#endif
#ifdef EPIPE
    CASE_STRERRNO(EPIPE)
#endif
#ifdef EPROTO
    CASE_STRERRNO(EPROTO)
#endif
#ifdef EPROTONOSUPPORT
    CASE_STRERRNO(EPROTONOSUPPORT)
#endif
#ifdef EPROTOTYPE
    CASE_STRERRNO(EPROTOTYPE)
#endif
#ifdef ERANGE
    CASE_STRERRNO(ERANGE)
#endif
#ifdef EREMCHG
    CASE_STRERRNO(EREMCHG)
#endif
#ifdef EREMOTE
    CASE_STRERRNO(EREMOTE)
#endif
#ifdef EREMOTEIO
    CASE_STRERRNO(EREMOTEIO)
#endif
#ifdef ERESTART
    CASE_STRERRNO(ERESTART)
#endif
#ifdef ERFKILL
    CASE_STRERRNO(ERFKILL)
#endif
#ifdef EROFS
    CASE_STRERRNO(EROFS)
#endif
#ifdef ESHUTDOWN
    CASE_STRERRNO(ESHUTDOWN)
#endif
#ifdef ESPIPE
    CASE_STRERRNO(ESPIPE)
#endif
#ifdef ESOCKTNOSUPPORT
    CASE_STRERRNO(ESOCKTNOSUPPORT)
#endif
#ifdef ESRCH
    CASE_STRERRNO(ESRCH)
#endif
#ifdef ESTALE
    CASE_STRERRNO(ESTALE)
#endif
#ifdef ESTRPIPE
    CASE_STRERRNO(ESTRPIPE)
#endif
#ifdef ETIME
    CASE_STRERRNO(ETIME)
#endif
#ifdef ETIMEDOUT
    CASE_STRERRNO(ETIMEDOUT)
#endif
#ifdef ETOOMANYREFS
    CASE_STRERRNO(ETOOMANYREFS)
#endif
#ifdef ETXTBSY
    CASE_STRERRNO(ETXTBSY)
#endif
#ifdef EUCLEAN
    CASE_STRERRNO(EUCLEAN)
#endif
#ifdef EUNATCH
    CASE_STRERRNO(EUNATCH)
#endif
#ifdef EUSERS
    CASE_STRERRNO(EUSERS)
#endif
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
    CASE_STRERRNO(EWOULDBLOCK)
#endif
#ifdef EXDEV
    CASE_STRERRNO(EXDEV)
#endif
#ifdef EXFULL
    CASE_STRERRNO(EXFULL)
#endif
    default:
        return NULL;
    }
}
