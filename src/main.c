//
//  main.c
//
//  Author:
//       Antonio Natale <inferdevil97@gmail.com>
//
//  Copyright (c) 2014 WareX
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <aplus.h>
#include <aplus/fs.h>
#include <aplus/list.h>
#include <grub.h>
#include <aplus/fs.h>
#include <aplus/task.h>
#include <aplus/attribute.h>
#include <aplus/mm.h>

#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/times.h>
#include <time.h>

#include <aplus/netif.h>
#include <aplus/net/eth.h>
#include <aplus/net/ipv4.h>
#include <aplus/net/ipv6.h>

#include <errno.h>


extern inode_t* vfs_root;
extern task_t* current_task;


/**
 *	\brief Put Kernel Task in optimized loop for powersave.
 */
static void sysidle() {
	schedule_setpriority(TASK_PRIORITY_MIN);
char* __argv[] = {
		"/dev/ramdisk/bin/init", 
		NULL
	};


	char* __envp[] = {
		"PATH=/bin:/usr/bin:/usr/local/bin:/dev/ramdisk/bin",
		"SHELL=/bin/dash",
		"USER=liveuser",
		"TMPDIR=/tmp",
		NULL
	};
//if(sys_fork() == 0)
		sys_execve(__argv[0], __argv, __envp);
	for(;;)
		__asm__ ("pause");
}


/**
 *	\brief Entry point for kernel.
 */
int main() {
	serial_init();
	mm_init();
	desc_init();
	syscall_init();
	vfs_init();
	schedule_init();
	pci_init();
	tty_init();
	netif_init();


	struct utsname u;
	sys_uname(&u);

	kprintf("%s %s %s %s %s\n", u.sysname, u.nodename, u.release, u.version, u.machine);

	if(mbd->mods_count == 0)
		panic("no initrd module found");
	

	uint32_t addr = ((uint32_t*) mbd->mods_addr) [0];
	uint32_t endp = ((uint32_t*) mbd->mods_addr) [1];

	kprintf("initrd: module found at addess: 0x%x (%d KB)\n", addr, (endp - addr) / 1024);


	if(!mkramdev("/dev/ram0", addr, endp - addr))
		panic("initrd: cannot create /dev/ram0");


	if(sys_mount("/dev/ram0", "/dev/ramdisk", "iso9660", 0, 0) != 0)
		panic("initrd: cannot mount ramdisk");


	if(sys_mount(NULL, "/dev/proc", "procfs", 0, 0) != 0)
		panic("procfs: cannot mount /dev/proc");

	if(sys_mount(NULL, "/dev/tmp", "tmpfs", 0, 0) != 0)
		panic("tmpfs: cannot mount /dev/tmp");

	if(sys_symlink("/dev/proc", "/proc") != 0)
		panic("procfs: cannot create link for /proc");

	if(sys_symlink("/dev/tmp", "/tmp") != 0)
		panic("tmpfs: cannot link for /tmp");


	kprintf("VRAM: 0x%x\n", mbd->vbe_mode_info->physbase);


	


	task_clone(sysidle, NULL, NULL, 0xFF);
for(;;) sys_yield();
	
	
}


