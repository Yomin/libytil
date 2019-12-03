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

#include <ytil/sys/fs.h>
#include <ytil/def.h>
#include <dirent.h>
#include <sys/stat.h>


static const error_info_st error_infos[] =
{
      ERROR_INFO(E_FS_ERRNO, "ERRNO wrapper.")
    , ERROR_INFO(E_FS_INVALID_PATH, "Invalid path.")
    , ERROR_INFO(E_FS_NOT_FOUND, "File not found.")
};


fs_stat_st *fs_stat(path_const_ct file, fs_flag_fs flags, fs_stat_st *fst)
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
    if(flags & FS_FLAG_NOFOLLOW)
        rc = lstat(str_c(path), &st);
    else
        rc = stat(str_c(path), &st);
#endif
    
    str_unref(path);
    
    if(rc) switch(errno)
    {
    case ENOENT:    return error_push_errno(E_FS_NOT_FOUND, stat), NULL;
    default:        return error_push_errno(E_FS_ERRNO, stat), NULL;
    }
    
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

int fs_walk(path_const_ct dir, fs_flag_fs flags, fs_walk_cb walk, void *ctx)
{
    path_ct path;
    str_ct str;
    DIR *dp;
    struct dirent *ep;
    fs_stat_st fst;
    int rc;
    
    assert(walk);
    
    if(!(path = path_dup(dir)))
        return error_wrap(), -1;
    
    if(!(str = path_get(path, PATH_STYLE_NATIVE)))
        return error_pack(E_FS_INVALID_PATH), path_free(path), -1;
    
    if(!(dp = opendir(str_c(str))))
        return error_push_errno(E_FS_ERRNO, opendir), str_unref(str), path_free(path), -1;
    
    str_unref(str);
    
    for(errno = 0; (ep = readdir(dp)); errno = 0)
    {
        if(!strcmp(ep->d_name, path_current(PATH_STYLE_NATIVE))
        || !strcmp(ep->d_name, path_parent(PATH_STYLE_NATIVE)))
            continue;
        
        if(!path_append_c(path, ep->d_name, PATH_STYLE_NATIVE))
            return error_wrap(), path_free(path), closedir(dp), -1;
        
        if(fs_stat(path, flags, &fst))
            return error_pass(), path_free(path), closedir(dp), -1;
        
        if((rc = walk(path, &fst, ctx)))
            return error_wrap(), path_free(path), closedir(dp), rc;
        
        path_drop(path, 1);
    }
    
    if(errno)
        return error_push_errno(E_FS_ERRNO, readdir), path_free(path), closedir(dp), -1;
    
    path_free(path);
    closedir(dp);
    
    return 0;
}

/*
int fs_move(path_const_ct dst, path_const_ct src, fs_mode_id mode)
{
    assert(mode < FS_MODES);
    
    if(!rename(str_c(src), str_c(dst)))
        return 0;
    
    if(errno != EEXIST && errno != ENOTEMPTY)
        return error_propagate_errno(), -1;
    
    if(mode == FS_MODE_REPLACE)
    {
        if(fs_remove(dst))
            return error_propagate(), -1;
        
        if(rename(str_c(src), str_c(dst)))
            return error_propagate_errno(), -1;
        
        return 0;
    }
    
    return errno_set(ENOSYS), -1; // todo
}

int fs_copy(path_const_ct dst, path_const_ct src, fs_mode_id mode)
{
    return 0;
}

int fs_remove(path_const_ct file)
{
    return 0;
}
*/
