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
#include <libc.h>

SYSCALL(804, fstatvfs,
int sys_fstatvfs(int fd, struct statvfs* st) {
    if(unlikely(!st)) {
        errno = EINVAL;
        return -1;
    }

    if(unlikely(fd < 0 || fd > TASK_FD_COUNT)) {
        errno = EBADF;
        return -1;
    }

    inode_t* inode = current_task->fd[fd].inode;
    if(unlikely(!inode)) {
        errno = EBADF;
        return -1;
    }

    if(unlikely(!inode->mtinfo)) {
        errno = ENOSYS;
        return -1;
    }

    memcpy(st, &inode->mtinfo->stat, sizeof(struct statvfs));
    return 0;
});