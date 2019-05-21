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
 * Name:        clock_settime
 * Description: clock and time functions
 * URL:         http://man7.org/linux/man-pages/man2/clock_settime.2.html
 *
 * Input Parameters:
 *  0: 0xe3
 *  1: clockid_t which_clock
 *  2: const struct timespec __user * tp
 *
 * Auto-generated by extra/utils/gen-syscalls.js
 */

SYSCALL(227, clock_settime,
long sys_clock_settime (clockid_t which_clock, const struct timespec __user * tp) {
    return -ENOSYS;
});
