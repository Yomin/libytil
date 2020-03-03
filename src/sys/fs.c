/*
 * Copyright (c) 2019-2020 Martin Rödel a.k.a. Yomin Nimoy
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

#include <ytil/sys/fs.h>
#include <ytil/def.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>


#ifndef O_BINARY
#   define O_BINARY 0
#endif

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_FS_ACCESS_DENIED, "Access denied.")
    , ERROR_INFO(E_FS_BUSY, "File is in use.")
    , ERROR_INFO(E_FS_CALLBACK, "Callback error.")
    , ERROR_INFO(E_FS_ERRNO, "ERRNO wrapper.")
    , ERROR_INFO(E_FS_INVALID_PATH, "Invalid path.")
    , ERROR_INFO(E_FS_NO_SPACE, "No space available.")
    , ERROR_INFO(E_FS_NOT_DIRECTORY, "Path is not a directory.")
    , ERROR_INFO(E_FS_NOT_FOUND, "File not found.")
};


static fs_error_id fs_get_errno_error(int error)
{
    switch(error)
    {
    case EPERM:
    case EACCES:    return E_FS_ACCESS_DENIED;
    case EBUSY:     return E_FS_BUSY;
    case ENOENT:    return E_FS_NOT_FOUND;
    case ENOSPC:    return E_FS_NO_SPACE;
    case ENOTDIR:   return E_FS_NOT_DIRECTORY;
    default:        return E_FS_ERRNO;
    }
}

#define fs_error_push_errno(sub) error_push_errno(fs_get_errno_error(errno), sub)

fs_stat_st *fs_stat(path_const_ct file, fs_stat_fs flags, fs_stat_st *fst)
{
    struct stat st;
    str_ct path;
    int rc;
    
    assert(fst);
    
    if(!(path = path_get(file, PATH_STYLE_NATIVE)))
        return error_pack(E_FS_INVALID_PATH), NULL;
    
#ifdef _WIN32
    rc = stat(str_c(path), &st);
#else
    if(flags & FS_STAT_LINK_NOFOLLOW)
        rc = lstat(str_c(path), &st);
    else
        rc = stat(str_c(path), &st);
#endif
    
    str_unref(path);
    
    if(rc)
        return fs_error_push_errno(stat), NULL;
    
    switch(st.st_mode & S_IFMT)
    {
    case S_IFBLK:   fst->type = FS_TYPE_BLOCK; break;
    case S_IFCHR:   fst->type = FS_TYPE_CHARACTER; break;
    case S_IFIFO:   fst->type = FS_TYPE_PIPE; break;
#ifndef _WIN32
    case S_IFSOCK:  fst->type = FS_TYPE_SOCKET; break;
    case S_IFLNK:   fst->type = FS_TYPE_LINK; break;
#endif
    case S_IFREG:   fst->type = FS_TYPE_REGULAR; break;
    case S_IFDIR:   fst->type = FS_TYPE_DIRECTORY; break;
    default:        fst->type = FS_TYPE_UNKNOWN;
    }
    
    fst->size = st.st_size;
    fst->uid = st.st_uid;
    fst->gid = st.st_gid;
    fst->atime = st.st_atime;
    fst->mtime = st.st_mtime;
    fst->ctime = st.st_ctime;
    
    return fst;
}

static int fs_walk_dir(path_ct path, ssize_t maxdepth, size_t depth, fs_stat_fs flags, fs_stat_st *fst_parent, fs_walk_cb walk, void *ctx)
{
    str_ct str;
    DIR *dp;
    struct dirent *ep;
    fs_stat_st fst_child;
    int rc;
    
    if((rc = walk(FS_WALK_DIR_PRE, path, depth, fst_parent, ctx)))
        return error_push_int(E_FS_CALLBACK, rc);
    
    if(maxdepth < 0 || depth < (size_t)maxdepth)
    {
        if(!(str = path_get(path, PATH_STYLE_NATIVE)))
            return error_pack(E_FS_INVALID_PATH), -1;
        
        dp = opendir(str_c(str));
        str_unref(str);
        
        if(!dp)
        {
            fs_error_push_errno(opendir);
            
            return error_push_int(E_FS_CALLBACK, walk(FS_WALK_DIR_ERROR, path, depth, fst_parent, ctx));
        }
        
        for(errno = 0; (ep = readdir(dp)); errno = 0)
        {
            if(!strcmp(ep->d_name, path_current(PATH_STYLE_NATIVE))
            || !strcmp(ep->d_name, path_parent(PATH_STYLE_NATIVE)))
                continue;
            
            if(!path_append_c(path, ep->d_name, PATH_STYLE_NATIVE))
                return error_wrap(), closedir(dp), -1;
            
            if(!fs_stat(path, flags, &fst_child))
            {
                error_pass();
                
                if((rc = walk(FS_WALK_STAT_ERROR, path, depth+1, NULL, ctx)))
                    return error_push_int(E_FS_CALLBACK, rc), closedir(dp), rc;
            }
            else if(fst_child.type != FS_TYPE_DIRECTORY)
            {
                if((rc = walk(FS_WALK_FILE, path, depth+1, &fst_child, ctx)))
                    return error_push_int(E_FS_CALLBACK, rc), closedir(dp), rc;
            }
            else
            {
                if((rc = fs_walk_dir(path, maxdepth, depth+1, flags, &fst_child, walk, ctx)))
                    return closedir(dp), rc;
            }
            
            path_drop(path, 1);
        }
        
        if(errno)
            return fs_error_push_errno(readdir), closedir(dp), -1;
        
        closedir(dp);
    }
    
    if((rc = walk(FS_WALK_DIR_POST, path, depth, fst_parent, ctx)))
        return error_push_int(E_FS_CALLBACK, rc);
    
    return 0;
}

int fs_walk(path_const_ct dir, ssize_t depth, fs_stat_fs flags, fs_walk_cb walk, void *ctx)
{
    fs_stat_st fst;
    path_ct path;
    int rc;
    
    assert(walk);
    
    if(!fs_stat(dir, flags, &fst))
    {
        error_pass();
        
        if(error_check(0, E_FS_INVALID_PATH))
            return -1;
        
        return error_push_int(E_FS_CALLBACK, walk(FS_WALK_STAT_ERROR, dir, 0, NULL, ctx));
    }
    
    if(fst.type != FS_TYPE_DIRECTORY)
        return error_push_int(E_FS_CALLBACK, walk(FS_WALK_FILE, dir, 0, &fst, ctx));
    
    if(!(path = path_dup(dir)))
        return error_wrap(), -1;
    
    rc = fs_walk_dir(path, depth, 0, flags, &fst, walk, ctx);
    path_free(path);
    
    return error_pass_int(rc);
}

int fs_move(path_const_ct src, path_const_ct dst, fs_move_fs flags)
{
    str_ct str_src, str_dst;
    
    if(!(str_src = path_get(src, PATH_STYLE_NATIVE)))
        return error_pack(E_FS_INVALID_PATH), -1;
    
    if(!(str_dst = path_get(dst, PATH_STYLE_NATIVE)))
        return error_pack(E_FS_INVALID_PATH), str_unref(str_src), -1;
    
    if(fs_remove(dst) && !error_check(0, E_FS_NOT_FOUND))
        return error_pass(), str_unref(str_src), str_unref(str_dst), -1;
    
    if(rename(str_c(str_src), str_c(str_dst)))
        return fs_error_push_errno(rename), str_unref(str_src), str_unref(str_dst), -1;
    
    str_unref(str_src);
    str_unref(str_dst);
    
    return 0;
}

static ssize_t fs_write(int fd, const void *vbuf, size_t size)
{
    ssize_t count;
    const char *buf = vbuf;
    
    while(size)
    {
        if((count = write(fd, buf, size)) < 0)
        {
            if(errno == EINTR)
                continue;
            
            return fs_error_push_errno(write), -1;
        }
        
        if(!count)
            return error_set(E_FS_NO_SPACE), -1;
        
        size -= count;
        buf += count;
    }
    
    return 0;
}

static int fs_copy_file(str_ct src, str_ct dst)
{
    int fd_src, fd_dst;
    char *buf;
    ssize_t size;
    
    if((fd_src = open(str_c(src), O_RDONLY|O_BINARY)) < 0)
        return fs_error_push_errno(open), -1;
    
    if((fd_dst = open(str_c(dst), O_WRONLY|O_BINARY|O_CREAT, S_IRWXU|S_IRWXG)) < 0)
        return fs_error_push_errno(open), close(fd_src), -1;
    
    if(!(buf = malloc(128*1024)))
        return fs_error_push_errno(malloc), close(fd_src), close(fd_dst), -1;
    
    while((size = read(fd_src, buf, 128*1024)))
    {
        if(size < 0)
        {
            if(errno == EINTR)
                continue;
            
            return fs_error_push_errno(read), close(fd_src), close(fd_dst), free(buf), -1;
        }
        
        if(fs_write(fd_dst, buf, size) < 0)
            return error_pass(), close(fd_src), close(fd_dst), free(buf), -1;
    }
    
    close(fd_src);
    close(fd_dst);
    free(buf);
    
    return 0;
}

int fs_copy(path_const_ct src, path_const_ct dst, fs_copy_fs flags)
{
    str_ct str_src, str_dst;
    
    if(!(str_src = path_get(src, PATH_STYLE_NATIVE)))
        return error_pack(E_FS_INVALID_PATH), -1;
    
    if(!(str_dst = path_get(dst, PATH_STYLE_NATIVE)))
        return error_pack(E_FS_INVALID_PATH), str_unref(str_src), -1;
    
    if(fs_copy_file(str_src, str_dst))
        return error_pass(), str_unref(str_src), str_unref(str_dst), -1;
    
    str_unref(str_src);
    str_unref(str_dst);
    
    return 0;
}

typedef struct fs_remove_state
{
    fs_walk_cb error;
    void *ctx;
} fs_remove_st;

static int fs_walk_remove(fs_walk_type_id type, path_const_ct file, size_t depth, fs_stat_st *info, void *ctx)
{
    fs_remove_st *state = ctx;
    str_ct str;
    int rc = -1;
    
    switch(type)
    {
    case FS_WALK_FILE:
        if(!(str = path_get(file, PATH_STYLE_NATIVE)))
            return error_wrap(), -1;
        
        if((rc = unlink(str_c(str))))
            fs_error_push_errno(unlink);
        
        str_unref(str);
        break;
    case FS_WALK_DIR_PRE:
        return 0;
    case FS_WALK_DIR_POST:
        if(!(str = path_get(file, PATH_STYLE_NATIVE)))
            return error_wrap(), -1;
        
        if((rc = rmdir(str_c(str))))
            fs_error_push_errno(rmdir);
        
        str_unref(str);
        break;
    case FS_WALK_DIR_ERROR:
    case FS_WALK_STAT_ERROR:
        error_pass();
        break;
    default:
        abort();
    }
    
    return !rc ? 0
        : !state || !state->error ? -1
        : error_push_int(E_FS_CALLBACK, state->error(type, file, depth, info, state->ctx));
}

int fs_remove(path_const_ct file)
{
    return error_lift_int(E_FS_CALLBACK, fs_walk(file, -1, FS_STAT_LINK_NOFOLLOW, fs_walk_remove, NULL));
}

int fs_remove_f(path_const_ct file, fs_walk_cb error, void *ctx)
{
    fs_remove_st state = { .error = error, .ctx = ctx };
    
    return error_lift_int(E_FS_CALLBACK, fs_walk(file, -1, FS_STAT_LINK_NOFOLLOW, fs_walk_remove, &state));
}
