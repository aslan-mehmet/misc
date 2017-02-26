/*
 * author: Mehmet ASLAN
 * date: February 26, 2017
 * no warranty, no licence agreement
 * use it at your own risk
 */

_estack = 0x20002000;

ENTRY(Reset_Handler);

MEMORY
{
FLASH (rx):	ORIGIN =	0x8000000,	LENGTH = 64K
RAM   (xrw):	ORIGIN =	0x20000000,	LENGTH = 8K
}

SECTIONS
{
	/* output section */
	.isr_vector :
	{
		/* builtin function */
		/* align location counter */
		. =  ALIGN(4);
		/* input section, protect from garbage collection */
		KEEP(*(.isr_vector))
	} > FLASH

	.text :
	{
		. = ALIGN(4);
		*(.text)
		*(.text*)
	} > FLASH

	.data :
	{
	} > RAM AT>FLASH

	.bss :
	{
	} > RAM
}
