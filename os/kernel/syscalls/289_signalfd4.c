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
 * Name:        signalfd4
 * Description: create a file descriptor for accepting signals
 * URL:         http://man7.org/linux/man-pages/man2/signalfd4.2.html
 *
 * Input Parameters:
 *  0: 0x121
 *  1: int ufd
 *  2: sigset_t __user * user_mask
 *  3: size_t sizemask
 *  4: int flags
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(289, signalfd4,
long sys_signalfd4 (int ufd, sigset_t __user * user_mask, size_t sizemask, int flags) {
    return -ENOSYS;
});
