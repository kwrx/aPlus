/*
 * Author:
 *      Antonino Natale <antonio.natale97@hotmail.com>
 * 
 * Copyright (c) 2013-2019 Antonino Natale
 * 
 * 
 * This file is part of aPlus.
 * 
 * aPlus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * aPlus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with aPlus.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/smp.h>
#include <aplus/vfs.h>
#include <aplus/mm.h>
#include <string.h>
#include <errno.h>


struct {
    int id;
    const char* name;
    int (*mount)(inode_t*, inode_t*, int, const char*);
    int (*umount) (inode_t*);
} vfs_table[32];


inode_t _vfs_root;
inode_t* vfs_root = &_vfs_root;

inode_t* vfs_dev = NULL;


void vfs_init(void) {
    
    memset(&_vfs_root, 0, sizeof(_vfs_root));
    memset(&vfs_table, 0, sizeof(vfs_table));

    int i = 0;
    #include "fstable.c.in"


    _vfs_root.name[0] = '/';
    _vfs_root.name[1] = '\0';

    _vfs_root.st.st_ino = 1;
    _vfs_root.st.st_mode = S_IFDIR;
    _vfs_root.st.st_nlink = 1;

    //_vfs_root.st.st_atime = arch_timer_now();
    //_vfs_root.st.st_ctime = arch_timer_now();
    //_vfs_root.st.st_mtime = arch_timer_now();

    spinlock_init(&_vfs_root.lock);

}



int vfs_mount(inode_t* dev, inode_t* dir, const char __user * fs, int flags, const char __user * args) {
    DEBUG_ASSERT(dir);
    DEBUG_ASSERT(fs);

    if(unlikely(!ptr_check(fs, R_OK)))
        return errno = EFAULT, -1;

    if(unlikely(args && !ptr_check(args, R_OK)))
        return errno = EFAULT, -1;

    
    int i;
    for(i = 0; vfs_table[i].id; i++) {
        if(strcmp(vfs_table[i].name, fs) != 0)
            continue;


        DEBUG_ASSERT(vfs_table[i].mount);
        DEBUG_ASSERT(vfs_table[i].umount);


        int e;
        if((e = vfs_table[i].mount(dev, dir, flags, args) < 0))
            return e;

        __lock(&dir->lock, {

            struct inode_ops ops;
            memcpy(&ops, &dir->ops, sizeof(ops));
            memcpy(&dir->ops, &dir->mount.ops, sizeof(ops));
            memcpy(&dir->mount.ops, &ops, sizeof(ops));

        });


        kprintf("mount: volume %s mounted on %s with %s\n", dev ? dev->name : "nodev", dir->name, fs);
        return 0;
    }

    return errno = EINVAL, -1;
}


int vfs_open(inode_t* inode) {
    DEBUG_ASSERT(inode);

    if(likely(inode->ops.open))
        __lock_return(&inode->lock, int, inode->ops.open(inode));

    return errno = ENOSYS, -1;
}

int vfs_close(inode_t* inode) {
    DEBUG_ASSERT(inode);

    if(likely(inode->ops.close))
        __lock_return(&inode->lock, int, inode->ops.close(inode));

    return errno = ENOSYS, -1;
}

int vfs_read(inode_t* inode, void __user * buf, off_t off, size_t size) {
    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(buf);

    if(unlikely(size == 0))
        return 0;

    if(unlikely(!ptr_check(buf, R_OK | W_OK)))
        return errno = EFAULT, -1;


    if(likely(inode->ops.read))
        __lock_return(&inode->lock, int, inode->ops.read(inode, buf, off, size));
    
    return errno = ENOSYS, -1;
}

int vfs_write(inode_t* inode, const void __user * buf, off_t off, size_t size) {
    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(buf);

    if(unlikely(size == 0))
        return 0;

    if(unlikely(!ptr_check(buf, R_OK)))
        return errno = EFAULT, -1;


    if(likely(inode->ops.write))
        __lock_return(&inode->lock, int, inode->ops.write(inode, buf, off, size));
    
    return errno = ENOSYS, -1;
}

inode_t* vfs_finddir(inode_t* inode, const char __user * name) {
    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(name);

    if(unlikely(!ptr_check(name, R_OK)))
        return errno = EFAULT, NULL;


    if(name[0] == '.' && name[1] == '\0')
        return inode;


    if(name[0] == '.' && name[1] == '.' && name[2] == '\0') {
       
        if(current_task->root != inode || inode->parent)
            return inode->parent;
        else
            return errno = ENOENT, NULL;

    }


    if(likely(inode->ops.finddir))
        __lock_return(&inode->lock, inode_t*, inode->ops.finddir(inode, name));
    
    return errno = ENOSYS, NULL;
}


inode_t* vfs_mknod(inode_t* inode, const char __user * name, mode_t mode) {
    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(name);

    if(unlikely(!ptr_check(name, R_OK)))
        return errno = EFAULT, NULL;


    if(name[0] == '.' && name[1] == '\0')
        return errno = EEXIST, NULL;

    if(name[0] == '.' && name[1] == '.' && name[2] == '\0')
        return errno = EEXIST, NULL;


    if(likely(inode->ops.mknod))
        __lock_return(&inode->lock, inode_t*, inode->ops.mknod(inode, name, mode));
    
    return errno = ENOSYS, NULL;
}

int vfs_readdir(inode_t* inode, struct dirent __user * ent, off_t off, size_t size) {
    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(ent);

    if(unlikely(size == 0))
        return 0;

    if(unlikely(!ptr_check(ent, R_OK | W_OK)))
        return errno = EFAULT, -1;


    if(likely(inode->ops.readdir))
        __lock_return(&inode->lock, int, inode->ops.readdir(inode, ent, off, size));
    
    return errno = ENOSYS, -1;
}

int vfs_unlink(inode_t* inode, const char __user * name) {
    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(name);

    if(unlikely(!ptr_check(name, R_OK)))
        return errno = EFAULT, -1;


    if(name[0] == '.' && name[1] == '\0')
        return errno = ENOTEMPTY, -1;

    if(name[0] == '.' && name[1] == '.' && name[2] == '\0')
        return errno = ENOTEMPTY, -1;


    if(likely(inode->ops.unlink))
        __lock_return(&inode->lock, int, inode->ops.unlink(inode, name));
    
    return errno = EROFS, -1;
}

int vfs_ioctl(inode_t* inode, int req, void __user * arg) {
    DEBUG_ASSERT(inode);

    if(unlikely(!ptr_check(arg, R_OK | W_OK)))
        return errno = EFAULT, -1;


    if(likely(inode->ops.ioctl))
        __lock_return(&inode->lock, int, inode->ops.ioctl(inode, req, arg));
    
    return errno = ENOSYS, -1;
}

int vfs_fsync(inode_t* inode) {
    DEBUG_ASSERT(inode);

    if(likely(inode->ops.fsync))
        __lock_return(&inode->lock, int, inode->ops.fsync(inode));
    
    return errno = ENOSYS, -1;
}