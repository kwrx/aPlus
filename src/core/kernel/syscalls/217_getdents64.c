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
#include <aplus/task.h>
#include <aplus/smp.h>
#include <aplus/errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/***
 * Name:        getdents64
 * Description: get directory entries
 * URL:         http://man7.org/linux/man-pages/man2/getdents64.2.html
 *
 * Input Parameters:
 *  0: 0xd9
 *  1: unsigned int fd
 *  2: struct linux_dirent64 __user * dirent
 *  3: unsigned int count
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

struct linux_dirent64;

SYSCALL(217, getdents64,
long sys_getdents64 (unsigned int fd, struct linux_dirent64 __user * dirent, unsigned int count) {
    return -ENOSYS;
});