/*
 * Author:
 *      Antonino Natale <antonio.natale97@hotmail.com>
 * 
 * Copyright (c) 2013-2018 Antonino Natale
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
#include <aplus/module.h>
#include <aplus/vfs.h>
#include <aplus/mm.h>
#include <libc.h>

#include <aplus/kmem.h>

/* Global Shared Memory */

MODULE_NAME("system/kmem");
MODULE_DEPS("");
MODULE_AUTHOR("Antonino Natale");
MODULE_LICENSE("GPL");

static int kmem_ioctl(struct inode* inode, int req, void* ptr) {
    #define cp(x)                   \
        if(unlikely(!x)) {          \
            errno = EINVAL;         \
            return -1;           \
        }
    
    switch(req) {
        case KMEMIOCTL_ALLOC:
            cp(ptr);
            *((uintptr_t*) ptr) = (uintptr_t) kmalloc(*((uintptr_t*) ptr), GFP_USER);
            
            break;
        case KMEMIOCTL_FREE:
            cp(ptr);
            kfree(ptr);
                
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    
    return 0;
}

int init(void) {
    inode_t* ino;
    if(unlikely((ino = vfs_mkdev("kmem", -1, S_IFCHR | 0440)) == NULL))
        return -1;


    ino->ioctl = kmem_ioctl;
    return 0;
}



int dnit(void) {
    return 0;
}
