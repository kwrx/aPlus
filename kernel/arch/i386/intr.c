#include <xdev.h>
#include <xdev/ipc.h>
#include <xdev/mm.h>
#include <xdev/debug.h>
#include <xdev/intr.h>
#include <xdev/syscall.h>
#include <xdev/task.h>
#include <libc.h>

#include <arch/i386/i386.h>



extern void idt_load();
extern void fork_handler(void*);
extern void yield_handler(void*);

extern struct {
	struct {
		uint16_t base_low;
		uint16_t selector;
		uint8_t unused;
		uint8_t flags;
		uint16_t base_high;
	} e[256];

	struct {
		uint16_t limit;
		uint32_t base;
	} p;
} IDT32;

extern struct {
	void* data;
	irq_handler_t* handler;
} IRQ32[16];

int current_irq = -1;


int intr_init() {
	#define _i(x)															\
		extern void isr##x (void*);											\
		IDT32.e[x].base_low = ((uintptr_t) isr##x) & 0xFFFF;				\
		IDT32.e[x].base_high = ((uintptr_t) isr##x >> 16) & 0xFFFF;			\
		IDT32.e[x].selector = 0x08;											\
		IDT32.e[x].unused = 0;												\
		IDT32.e[x].flags = 0x8E


	_i(0);
	_i(1);
	_i(2);
	_i(3);
	_i(4);
	_i(5);
	_i(6);
	_i(7);
	_i(8);
	_i(9);
	_i(10);
	_i(11);
	_i(12);
	_i(13);
	_i(14);
	_i(15);
	_i(16);
	_i(17);
	_i(18);
	_i(19);
	_i(20);
	_i(21);
	_i(22);
	_i(23);
	_i(24);
	_i(25);
	_i(26);
	_i(27);
	_i(28);
	_i(29);
	_i(30);
	_i(31);
	_i(0x7F);
	_i(0x80);

	__asm__ __volatile__ ("lidt [eax]" : : "a" (&IDT32.p));



	#undef _i
	#define _i(x)																\
		extern void irq##x (void*);												\
		IDT32.e[x + 32].base_low = ((uintptr_t) irq##x) & 0xFFFF;				\
		IDT32.e[x + 32].base_high = ((uintptr_t) irq##x >> 16) & 0xFFFF;		\
		IDT32.e[x + 32].selector = 0x08;										\
		IDT32.e[x + 32].unused = 0;												\
		IDT32.e[x + 32].flags = 0x8E


	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x00);
	outb(0xA1, 0x00);
	

	_i(0);
	_i(1);
	_i(2);
	_i(3);
	_i(4);
	_i(5);
	_i(6);
	_i(7);
	_i(8);
	_i(9);
	_i(10);
	_i(11);
	_i(12);
	_i(13);
	_i(14);
	_i(15);

	__asm__ __volatile__ ("sti");
	return E_OK;
}

static int intr_sync = 0;

void intr_enable(void) {
	if(intr_sync <= 1)
		__asm__ __volatile__ ("sti"); 
	else
		intr_sync--;
}

void intr_disable(void) {
	uintptr_t flags;
	__asm__ __volatile__ (
		"pushf		\n"
		"pop eax	\n"
		: "=a"(flags)
	);


	if(flags & (1 << 9))
		intr_sync = 1;
	else
		intr_sync++;
		
	__asm__ __volatile__("cli");
}


void irq_enable(int number, irq_handler_t handler) {
	KASSERT(number >= 0 && number < 16);

	IRQ32[number].data = NULL;
	IRQ32[number].handler = handler;
}

void irq_disable(int number) {
	KASSERT(number >= 0 && number < 16);

	IRQ32[number].data = NULL;
	IRQ32[number].handler = NULL;
}

void* irq_set_data(int number, void* data) {
	KASSERT(number >= 0 && number < 16);

	void* tmp = IRQ32[number].data;
	IRQ32[number].data = data;

	return tmp;
}

void* irq_get_data(int number) {
	KASSERT(number >= 0 && number < 16);

	return IRQ32[number].data;
}

void irq_ack(int irq_no) {
	if(current_irq == -1)
		return;
		
	if(irq_no >= 8)
		outb(0xA0, 0x20);

	outb(0x20, 0x20);
	current_irq = -1;
}


void isr_handler(i386_context_t* context) {
#if DEBUG
	static char *exception_messages[] = {
		"Division By Zero",
		"Debug",
		"Non Maskable Interrupt",
		"Breakpoint",
		"Into Detected Overflow",
		"Out of Bounds",
		"Invalid Opcode",
		"No Coprocessor",

		"Double Fault",
		"Coprocessor Segment Overrun",
		"Bad TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection Fault",
		"Page Fault",
		"Unknown Interrupt",

		"Coprocessor Fault",
		"Alignment Check",
		"Machine Check",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",

		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved"
	};
#endif

	switch(context->int_no) {
		case 0x80:
			context->eax = syscall_handler(context->eax, context->ebx, context->ecx, context->edx, context->esi, context->edi);
			context->edx = errno;
			return;
		case 0x7F:
			switch(context->eax) {
				case 0:
					fork_handler(context);
					return;
				case 1:
					schedule_yield();
					return;
			}
			return;
		default:
			break;
	}


	kprintf(ERROR, "Exception! %s (%x:%x:%x) 0x%x\n", exception_messages[context->int_no], context->err_code & 1, (context->err_code >> 1) & 3, (context->err_code >> 3) & 0xFFF1);
	

	__asm__ ("cli");
	for(;;);
}


void irq_handler(i386_context_t* context) {
	int irq_no = context->int_no - 32;
	current_irq = irq_no;

	if(likely(IRQ32[irq_no].handler))
		IRQ32[irq_no].handler ((void*) context);

	irq_ack(irq_no);
}

EXPORT(irq_enable);
EXPORT(irq_disable);
EXPORT(irq_set_data);
EXPORT(irq_get_data);
EXPORT(irq_ack);

EXPORT(intr_enable);
EXPORT(intr_disable);