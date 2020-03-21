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
#include <aplus/syscall.h>
#include <aplus/memory.h>
#include <aplus/vfs.h>
#include <aplus/smp.h>
#include <aplus/errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <hal/cpu.h>
#include <hal/vmm.h>
#include <hal/debug.h>


/***
 * Name:        write
 * Description: write to a file descriptor
 * URL:         http://man7.org/linux/man-pages/man2/write.2.html
 *
 * Input Parameters:
 *  0: 0x01
 *  1: unsigned int fd
 *  2: const char __user * buf
 *  3: size_t size
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(1, write,
long sys_write (unsigned int fd, const void __user * buf, size_t size) {
    
    if(unlikely(!ptr_check(buf, R_OK)))
        return -EFAULT;

    if(unlikely(fd > OPEN_MAX)) /* TODO: Add Network Support */
        return -EBADF;

    if(unlikely(!current_task->fd[fd].ref))
        return -EBADF;

    if(unlikely(!(
        (current_task->fd[fd].flags & O_WRONLY) ||
        (current_task->fd[fd].flags & O_RDWR)
    )))
        return -EPERM;


    // TODO */
    // // if(unlikely(current_task->fd[fd].flags & O_NONBLOCK)) {
    // //     struct pollfd p;
    // //     p.fd = fd;
    // //     p.events = POLLOUT;
    // //     p.revents = 0;

    // //     if(sys_poll(&p, 1, 0) < 0)
    // //         return -EIO;

    // //     if(!(p.revents & POLLOUT))
    // //         return -EAGAIN;
    // // }


    current_task->iostat.wchar += (uint64_t) size;
    current_task->iostat.syscw += 1;


    int e = 0;

    __lock(&current_task->fd[fd].ref->lock, {

        if((e = vfs_write(current_task->fd[fd].ref->inode, buf, current_task->fd[fd].ref->position, size)) <= 0)
            break;

        current_task->fd[fd].ref->position += e;
        current_task->iostat.write_bytes += (uint64_t) e;
        
    });


    if(e < 0)
        return -errno;

    return e;
});
