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
 * Name:        mq_open
 * Description: open a message queue
 * URL:         http://man7.org/linux/man-pages/man2/mq_open.2.html
 *
 * Input Parameters:
 *  0: 0xf0
 *  1: const char __user * name
 *  2: int oflag
 *  3: umode_t mode
 *  4: struct mq_attr __user * attr
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

struct mq_attr;

SYSCALL(240, mq_open,
long sys_mq_open (const char __user * name, int oflag, mode_t mode, struct mq_attr __user * attr) {
    return -ENOSYS;
});
