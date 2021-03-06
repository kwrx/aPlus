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
#include <aplus/smp.h>
#include <aplus/syscall.h>
#include <aplus/memory.h>
#include <aplus/task.h>
#include <aplus/ipc.h>
#include <aplus/elf.h>
#include <aplus/errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aplus/hal.h>




#define AT_NULL                 0	/* end of vector */
#define AT_IGNORE               1	/* entry should be ignored */
#define AT_EXECFD               2	/* file descriptor of program */
#define AT_PHDR                 3	/* program headers for program */
#define AT_PHENT                4	/* size of program header entry */
#define AT_PHNUM                5	/* number of program headers */
#define AT_PAGESZ               6	/* system page size */
#define AT_BASE                 7	/* base address of interpreter */
#define AT_FLAGS                8	/* flags */
#define AT_ENTRY                9	/* entry point of program */
#define AT_NOTELF               10	/* program is not ELF */
#define AT_UID                  11	/* real uid */
#define AT_EUID                 12	/* effective uid */
#define AT_GID                  13	/* real gid */
#define AT_EGID                 14	/* effective gid */
#define AT_PLATFORM             15  /* string identifying CPU for optimizations */
#define AT_HWCAP                16  /* arch dependent hints at CPU capabilities */
#define AT_CLKTCK               17	/* frequency at which times() increments */
#define AT_SECURE               23  /* secure mode boolean */
#define AT_BASE_PLATFORM        24	/* string identifying real platform, may differ from AT_PLATFORM. */
#define AT_RANDOM               25	/* address of 16 random bytes */
#define AT_HWCAP2               26	/* extension of AT_HWCAP */
#define AT_EXECFN               31	/* filename of program */




static inline uintptr_t __sbrk(uintptr_t incr) {

    uintptr_t brk = sys_brk(0);
    uintptr_t new = sys_brk(brk + incr);

    if(new == brk && incr)
         kpanicf("sys_execve(): out of memory!\n");

    return brk;

}



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

    if(unlikely(!uio_check(filename, R_OK)))
        return -EFAULT;

    if(unlikely(!uio_check(argv, R_OK)))
        return -EFAULT;

    if(unlikely(!uio_check(envp, R_OK)))
        return -EFAULT;


    int e;

    struct stat st;
    if((e = sys_newstat(filename, &st)) < 0)
        return e;



    e = 0;

    if(st.st_mode & S_IXUSR)
        e = (st.st_uid == current_task->uid);

    else if(st.st_mode & S_IXGRP)
        e = (st.st_gid == current_task->gid);
    
    else if(st.st_mode & S_IXOTH)
        e = 1;


    if(!e)
        return -EPERM;




    int fd;
    if((fd = sys_open(filename, O_RDONLY, 0)) < 0)
        return fd;


    Elf_Ehdr head;
    if((e = sys_read(fd, &head, sizeof(head))) < 0)
        return e;



    if(head.e_ident[0] == '#' &&
       head.e_ident[1] == '!')
        return -ENOEXEC; // TODO: Read Scripts


    if(
        (head.e_ident[EI_MAG0] != ELFMAG0) ||
        (head.e_ident[EI_MAG1] != ELFMAG1) ||
        (head.e_ident[EI_MAG2] != ELFMAG2) ||
        (head.e_ident[EI_MAG3] != ELFMAG3) ||
        (head.e_type != ET_EXEC)           ||
        (head.e_entry == 0)
    ) return -ENOEXEC;



  
    do_unshare(CLONE_FS);
    do_unshare(CLONE_FILES);
    do_unshare(CLONE_SIGHAND);
    do_unshare(CLONE_VM);


    #define RXX(a, b, c) {                                                  \
        if((e = sys_lseek(fd, (off_t) (b), SEEK_SET)) < 0)                  \
            return e;                                                       \
                                                                            \
        if((e = sys_read(fd, (void*)(a), (size_t)(c))) != (size_t)(c)) {    \
            if(e < 0)                                                       \
                return e;                                                   \
            else                                                            \
                return -EIO;                                                \
        }                                                                   \
    }



    uintptr_t end;
    uintptr_t flags;

    __lock(&current_task->lock, {


        //arch_userspace_release(); // TODO

        current_task->userspace.start = ~0UL;
        current_task->userspace.end   =  0UL;


        int i;
        for(i = 0; i < head.e_phnum; i++) {

            Elf_Phdr phdr;
            RXX(&phdr, head.e_phoff + (i * head.e_phentsize), head.e_phentsize);


            switch(phdr.p_type) {

                case PT_LOAD:                    
                case PT_TLS:

                    DEBUG_ASSERT(phdr.p_vaddr);
                    DEBUG_ASSERT(phdr.p_memsz);
                    DEBUG_ASSERT(phdr.p_filesz <= phdr.p_memsz);


                    end = phdr.p_vaddr + phdr.p_memsz;
                    end = (end & ~(phdr.p_align - 1)) + phdr.p_align;


                    if(phdr.p_vaddr < current_task->userspace.start)
                        current_task->userspace.start = phdr.p_vaddr;

                    if(end > current_task->userspace.end)
                        current_task->userspace.end = end;


                    flags = 0;
                    
                    if(!(phdr.p_flags & PF_X))
                        flags |= ARCH_VMM_MAP_NOEXEC;
                    
                    //if( (phdr.p_flags & PF_W))  // FIXME: musl write on non-writable segment on pthread_create();
                        flags |= ARCH_VMM_MAP_RDWR;


                    arch_vmm_map (current_task->address_space, phdr.p_vaddr, -1, phdr.p_memsz,
                                    ARCH_VMM_MAP_RDWR       |
                                    ARCH_VMM_MAP_TYPE_PAGE);


#if defined(DEBUG) && DEBUG_LEVEL >= 4
                    kprintf("sys_execve: PT_LOAD at address(%p) size(%p) alignsize(%p)\n", phdr.p_vaddr, phdr.p_memsz, end - phdr.p_vaddr);
#endif

                    RXX(phdr.p_vaddr, phdr.p_offset, phdr.p_filesz);


                    arch_vmm_mprotect (current_task->address_space, phdr.p_vaddr, phdr.p_memsz,
                                    flags                   |
                                    ARCH_VMM_MAP_USER);

                    break;


                case PT_GNU_EH_FRAME:
                    
                    kprintf("execve: WARN! PT_GNU_EH_FRAME not yet supported\n");
                    break;

                case PT_DYNAMIC:

                    kpanicf("execve: PANIC! PT_DYNAMIC not supported at kernel-level");
                    break;

                default:
                    continue;

            }
        }

        current_task->userspace.end = (current_task->userspace.end & ~(arch_vmm_getpagesize() - 1)) + arch_vmm_getpagesize();

    });


    DEBUG_ASSERT(current_task->userspace.start);
    DEBUG_ASSERT(current_task->userspace.end);
    DEBUG_ASSERT(current_task->userspace.start < current_task->userspace.end);
    DEBUG_ASSERT(current_task->fd->descriptors[fd].ref);

    sys_close(fd);



    int i;
    for(i = 0; i < CONFIG_OPEN_MAX; i++) {

        if(!current_task->fd->descriptors[i].ref)
            continue;

        if(!current_task->fd->descriptors[i].close_on_exec)
            continue;

        
        sys_close(i);

    }




    const char** __safe_argv = uio_get_ptr(argv);
    const char** __safe_envp = uio_get_ptr(envp);


#if defined(DEBUG) && DEBUG_LEVEL >= 0

    for(i = 0; __safe_argv[i]; i++)
        DEBUG_ASSERT(uio_check(__safe_argv[i], R_OK));

    for(i = 0; __safe_envp[i]; i++)
        DEBUG_ASSERT(uio_check(__safe_envp[i], R_OK));

#endif


    size_t argc;
    for(argc = 0; __safe_argv[argc]; argc++)
        ;

    size_t envc;
    for(envc = 0; __safe_envp[envc]; envc++)
        ;
        

        


    // * Prepare data for _start()
    
    char** argq = (char**) __builtin_alloca(sizeof(char*) * (argc + 1));
    char** envq = (char**) __builtin_alloca(sizeof(char*) * (envc + 1));


    // * Allocate ARGV e ENVP Strings

    for(i = 0; __safe_argv[i]; i++) {
     
        char* p = (char*) __sbrk(uio_strlen(__safe_argv[i]) + 1);
        uio_strcpy_u2u(p, __safe_argv[i]);

        *argq++ = p;
        *argq   = NULL;

    }

    for(i = 0; __safe_envp[i]; i++) {
     
        char* p = (char*) __sbrk(uio_strlen(__safe_envp[i]) + 1);
        uio_strcpy_u2u(p, __safe_envp[i]);

        *envq++ = p;
        *envq   = NULL;
    
    }



    // * Allocate Signal Stack
    
    __sbrk(SIGSTKSZ);
    uintptr_t sigstack = __sbrk(0);
    uintptr_t siginfo  = __sbrk(sizeof(siginfo_t));


    // * Allocate User Stack

    uintptr_t bottom = __sbrk(current_task->rlimits[RLIMIT_STACK].rlim_cur);
    uintptr_t stack  = __sbrk(0);


    uintptr_t* sp = (uintptr_t*) bottom;


    // * AUX vector

    #define AUX_ENT(id, value) {    \
        uio_wptr(sp++, id);         \
        uio_wptr(sp++, value);      \
    }


    AUX_ENT(AT_PAGESZ, arch_vmm_getpagesize());
    AUX_ENT(AT_FLAGS, 0);
    AUX_ENT(AT_ENTRY, head.e_entry);
    AUX_ENT(AT_UID, current_task->uid);
    AUX_ENT(AT_GID, current_task->gid);
    AUX_ENT(AT_EUID, current_task->euid);
    AUX_ENT(AT_EGID, current_task->egid);
    AUX_ENT(AT_CLKTCK, arch_timer_generic_getres());
    AUX_ENT(AT_RANDOM, rand());
    AUX_ENT(AT_HWCAP, 0);
    AUX_ENT(AT_NULL, 0);



    // * Environment

    uio_wptr(sp++, 0UL);
    
    for(i = envc - 1; i >= 0; i--)
        uio_wptr(sp++, (uintptr_t) envp[i]);

    uio_wptr(sp++, 0UL);


    // * Arguments

    for(i = argc - 1; i >= 0; i--)
        uio_wptr(sp++, (uintptr_t) argv[i]);

    uio_wptr(sp, argc);




    current_task->userspace.stack    = stack;
    current_task->userspace.sigstack = sigstack;
    current_task->userspace.siginfo  = (siginfo_t*) siginfo;


    arch_userspace_enter(head.e_entry, stack, sp);
    return -EINTR;
    
});
