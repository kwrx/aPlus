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
#include <aplus/vfs.h>
#include <aplus/task.h>
#include <aplus/ipc.h>
#include <aplus/syscall.h>
#include <aplus/debug.h>
#include <libc.h>

SYSCALL(9, lseek,
off_t sys_lseek(int fd, off_t off, int dir) {
    if(unlikely(fd >= TASK_FD_COUNT || fd < 0)) {
        errno = EBADF;
        return -1;
    }

    inode_t* inode = current_task->fd[fd].inode;

    if(unlikely(!inode)) {
        errno = EBADF;
        return -1;
    }


    if(S_ISFIFO(inode->mode)) {
        errno = ESPIPE;
        return -1;
    }

    switch(dir) {
        case SEEK_SET:
            current_task->fd[fd].position = off;
            break;
        case SEEK_CUR:
            current_task->fd[fd].position += off;
            break;
        case SEEK_END:
            current_task->fd[fd].position = off + inode->size;
            break;
        default:
            errno = EINVAL;
            return -1;
    }

    return current_task->fd[fd].position;
});
