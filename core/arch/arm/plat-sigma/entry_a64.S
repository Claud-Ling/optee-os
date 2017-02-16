#include <platform_config.h>

#include <asm.S>
#include <arm.h>

/*
 * unsigned long cpu_on_entry(unsigned long a0, unsigned long a1);
 * Entry point for secondary cores
 * Clobber list: x21, x22, x30
 * Note that: x19,x20,x21, and x30 are clobbered by cpu_on_handler,
 * where x19-x20 are used to save x0-x1, and x21 are used to save x30.
 */
.section .text.cpu_on_entry
FUNC cpu_on_entry , :
	mov	x22, x30
	bl	cpu_on_handler

	/* give platform a chance to do sth there */
	mov	x21, x0
	mov	x0, x19
	mov	x1, x20
	bl	cpu_on_main
	mov	x0, x21
	ret	x22
END_FUNC cpu_on_entry
