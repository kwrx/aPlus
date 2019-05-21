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
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/***
 * Name:        pivot_root
 * Description: change the root filesystem
 * URL:         http://man7.org/linux/man-pages/man2/pivot_root.2.html
 *
 * Input Parameters:
 *  0: 0x9b
 *  1: const char __user * new_root
 *  2: const char __user * put_old
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(155, pivot_root,
long sys_pivot_root (const char __user * new_root, const char __user * put_old) {
    return -ENOSYS;
});
