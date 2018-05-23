#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/vfs.h>
#include <aplus/mm.h>
#include <aplus/task.h>
#include <libc.h>

#include "procfs.h"


int procfs_mount(struct inode* dev, struct inode* dir, struct mountinfo* info) {
    (void) dev;


    dir->open = procfs_open;
    dir->close = NULL;
    dir->finddir = procfs_finddir;
    dir->mknod = NULL;
    dir->unlink = NULL;
    dir->read = NULL;
    dir->write = NULL;
    dir->ioctl = NULL;
    dir->rename = NULL;
    dir->mtinfo = info;

    info->stat.f_bsize = PAGE_SIZE;
    info->stat.f_frsize = PAGE_SIZE;
    info->stat.f_blocks = 0;
    info->stat.f_bfree = 0;
    info->stat.f_bavail = 0;
    info->stat.f_files = 0;
    info->stat.f_ffree = 0;
    info->stat.f_favail = 0;
    info->stat.f_namemax = UINT32_MAX;


    procfs_entry_t* sb = (procfs_entry_t*) kmalloc(sizeof(procfs_entry_t), GFP_KERNEL);
    if(unlikely(!sb)) {
        kprintf(ERROR "procfs: no memory left!\n");
        return -1;
    }

    memset(sb, 0, sizeof(sb));
    strcpy(sb->name, "proc");
    sb->task = NULL;
    sb->mode = S_IFDIR;
    sb->parent = NULL;
    sb->init = procfs_init;
    sb->update = procfs_update;
    dir->userdata = (void*) sb;
    

    sb->init(sb);
    return 0;
}