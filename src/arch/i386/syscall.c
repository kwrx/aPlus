#ifdef __i386__

#include <aplus.h>
#include <aplus/spinlock.h>
#include <aplus/syscall.h>
#include <aplus/attribute.h>

#include <errno.h>

#include <arch/i386/i386.h>


#define NSYSCALLS		1024

static void** syscall_handlers = NULL;


int syscall_init() {
	list_t* syslist = attribute("syscall");

	syscall_handlers = (void**) kmalloc(sizeof(void*) * NSYSCALLS);
	memset(syscall_handlers, 0, sizeof(void*) * NSYSCALLS);

	list_foreach(value, syslist) {
		syscall_t* sys = (syscall_t*) value;

		if(unlikely(syscall_handlers[sys->number]))
			kprintf("syscall: duplicate number of %d handler\n");

		syscall_handlers[sys->number] = sys->handler;
	}

	kprintf("syscall: loaded %d handlers\n", syslist->size);
	list_destroy(syslist);


#ifdef SYSCALL_DEBUG
	

	for(int i = 0, s = 0; i < NSYSCALLS; i++) {
		if(syscall_handlers[i] == NULL) {
			if(!s)
				continue;

			if(s > 1)
				kprintf("\t%d..%d\n", i - s, i - 1);
			else
				kprintf("\t%d\n", i - s);

			s = 0;
		} else
			s++;
	}
#endif


	return 0;
}

int syscall_invoke(int idx, int p0, int p1, int p2, int p3, int p4) {
	void* handler = syscall_handlers[idx];	

	if(unlikely(handler == NULL)) {
		errno = ENOSYS;
		return -1;
	}


#ifdef SYSCALL_DEBUG
	kprintf("syscall: pid(%d) call %s [%d] (%x, %x, %x, %x, %x);\n", sys_getpid(), exec_symbol_lookup(handler), idx, p0, p1, p2, p3, p4);
#endif



	int r = 0;

	__asm__ __volatile__ (
		"push ebx			\n"
		"push ecx			\n"
		"push edx			\n"
		"push esi			\n"
		"push edi			\n"
		"call eax			\n"
		"add esp, 20		\n"
		: "=a"(r) 
		: "a"(handler), "b"(p4), "c"(p3), "d"(p2), "S"(p1), "D"(p0)
	);

#ifdef SYSCALL_DEBUG
	kprintf("% returned %x\n", r);
#endif

	return r;
}


int syscall_handler(regs_t* r) {
	return syscall_invoke(r->eax, r->ebx, r->ecx, r->edx, r->esi, r->edi);
}


#endif
