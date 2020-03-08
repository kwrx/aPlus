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
#include <aplus/ipc.h>
#include <aplus/vfs.h>
#include <aplus/memory.h>
#include <aplus/errno.h>
#include <stdint.h>
#include <stdio.h>


#include "tmpfs.h"



int tmpfs_truncate(inode_t* inode, off_t len) {
    
    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(inode->sb);
    DEBUG_ASSERT(inode->sb->fsid == TMPFS_ID);



    tmpfs_inode_t* i = (tmpfs_inode_t*) vfs_cache_get(&inode->sb->cache, inode->ino);

    if(len >= i->st.st_size)
        return 0;
    


    i->capacity = BUFSIZ + len;
    i->st.st_size = len;

    i->data = krealloc(i->data, i->capacity, GFP_KERNEL);


    inode->sb->st.f_bfree -= i->st.st_size - len;
    inode->sb->st.f_bavail -= i->st.st_size - len;
    
    
    return 0;
}