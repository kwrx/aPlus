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
#include <aplus/mm.h>
#include <stdint.h>
#include <errno.h>

#include "ext2.h"


__thread_safe
inode_t* ext2_finddir(inode_t* inode, const char * name) {

    DEBUG_ASSERT(inode);
    DEBUG_ASSERT(inode->fsinfo);
    DEBUG_ASSERT(name);

    ext2_t* ext2 = (ext2_t*) inode->fsinfo;
    inode_t* child = NULL;


    struct ext2_inode node;
    ext2_utils_read_inode(ext2, inode->st.st_ino, &node);


    int q;
    for(q = 0; q < node.i_size; q += ext2->blocksize) {


        __lock(&ext2->lock, {
        
            ext2_utils_read_inode_data(ext2, node.i_block, q / ext2->blocksize, 0, ext2->cache, ext2->blocksize);


            int i;
            for(i = 0; i < ext2->blocksize; ) {

                struct ext2_dir_entry_2* d = (struct ext2_dir_entry_2*) ((uintptr_t) ext2->cache + i);


                /* Found? */
                if(strncmp(name, d->name, d->name_len) == 0) {

                    struct ext2_inode node;
                    ext2_utils_read_inode(ext2, d->inode, &node);


                    child = vfs_cache_get_inode(ext2->root->mount.cache, d->inode);

                    child->st.st_dev = 0;
                    child->st.st_mode = node.i_mode;
                    child->st.st_nlink = node.i_links_count;
                    child->st.st_uid = node.i_uid;
                    child->st.st_gid = node.i_gid;
                    child->st.st_rdev = 0;
                    child->st.st_size = node.i_size;
                    child->st.st_blksize = 512;
                    child->st.st_blocks = node.i_blocks;
                    child->st.st_atime = node.i_atime;
                    child->st.st_ctime = node.i_ctime;
                    child->st.st_mtime = node.i_mtime;

#if CONFIG_BITS == 64
                    child->st.st_size |= ((off_t) node.i_size_high) << 32;
#endif

                    //child->ops.fsync = ext2_fsync;

                    if(S_ISDIR(node.i_mode)) {

                        child->ops.finddir = ext2_finddir;
                        child->ops.readdir = ext2_readdir;
                        child->ops.mknod = ext2_mknod;
                        child->ops.unlink = ext2_unlink;

                    }

                    if(S_ISREG(node.i_mode)) {

                        child->ops.read = ext2_read;
                        child->ops.write = ext2_write;

                    }

                    strncpy(child->name, name, MAXNAMLEN);


                    child->fsinfo = inode->fsinfo;
                    child->userdata = NULL;
                    child->parent = inode->parent;
                    child->root = ext2->root;
                    
                    spinlock_init(&child->lock);


                    break;
                }
                    
                
                
                i += d->rec_len;
            }

        });


        if(child != NULL)
            break;

    }

    return child;
}