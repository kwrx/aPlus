#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/vfs.h>
#include <aplus/mm.h>
#include <libc.h>

#include "tmpfs.h"


int tmpfs_unlink(struct inode* inode, char* name) {
    if(inode->childs) {
        struct inode_childs* tmp;
        for(tmp = inode->childs; tmp; tmp = tmp->next) {
            if(strcmp(tmp->inode->name, name) == 0) {
                kfree((void*) tmp->inode->userdata);
                
                inode->mtinfo->stat.f_bfree += inode->size;
                inode->mtinfo->stat.f_bavail += inode->size;
                inode->mtinfo->stat.f_ffree++;
                inode->mtinfo->stat.f_favail++;


                return 0;
            }
        }
    }

    return -1;
}