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

#define _GNU_SOURCE
#include <sched.h>

#include <aplus.h>
#include <aplus/debug.h>
#include <aplus/syscall.h>
#include <aplus/mm.h>
#include <aplus/smp.h>
#include <aplus/task.h>
#include <aplus/ipc.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/***
 * Name:        execve
 * Description: execute program
 * URL:         http://man7.org/linux/man-pages/man2/execve.2.html
 *
 * Input Parameters:
 *  0: 0x3B
 *  1: const char __user *
 *  2: const char __user **
 *  3: const char __user **
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */


SYSCALL(59, execve,
long sys_execve (const char __user * filename, const char __user ** argv, const char __user ** envp) {
    
    if(unlikely(!filename))
        return -EINVAL;

    if(unlikely(!argv))
        return -EINVAL;

    if(unlikely(!envp))
        return -EINVAL;

    if(unlikely(!ptr_check(filename, R_OK)))
        return -EFAULT;

    if(unlikely(!ptr_check(argv, R_OK)))
        return -EFAULT;

    if(unlikely(!ptr_check(envp, R_OK)))
        return -EFAULT;


    
    /* TODO: Free data  */
    /* TODO: Add binfmt */


    return -ENOSYS;
});