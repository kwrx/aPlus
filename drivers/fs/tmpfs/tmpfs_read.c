#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/vfs.h>
#include <aplus/mm.h>
#include <libc.h>

#include "tmpfs.h"


int tmpfs_read(struct inode* inode, void* ptr, off_t pos, size_t len) {
    if(unlikely(!inode))
        return 0;

    if(unlikely(!ptr))
        return 0;

    if(unlikely(!inode->userdata))
        return 0;

    if(pos + len > inode->size)
        len = inode->size - pos;

    if(unlikely(!len))
        return 0;

    memcpy(ptr, (void*) ((uintptr_t) inode->userdata + (uintptr_t) pos), len);
    return len;
}