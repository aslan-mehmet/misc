include makefile.conf
NAME=main
STARTUP_DEFS=-D__STARTUP_CLEAR_BSS -D__START=main -D STM32F051 \
-D HSE_VALUE=8000000 -D USE_STDPERIPH_DRIVER -D USART1_INTERRUPT

LDSCRIPTS=-L. -T STM32F051R8_FLASH.ld
LFLAGS=$(USE_NANO) $(USE_NOHOST) $(LDSCRIPTS) $(GC) $(MAP)

STDPERIPH_SRC=/home/mehmet/Documents/arm_tools/STM32F0xx_StdPeriph_Lib_V1.5.0/Libraries/STM32F0xx_StdPeriph_Driver/src

$(NAME).elf: ../src/$(NAME).c $(STARTUP) \
	../src/*.c \
	$(STDPERIPH_SRC)/*.c
	$(CC) $^ $(CFLAGS) $(LFLAGS) -o $@

clean:
	rm -f $(NAME)*.elf *.map *.o *.bin

# make creates executable. binary command gives actual memory
# also can use gdb to flash .elf file
binary:
	arm-none-eabi-objcopy -O binary $(NAME).elf $(NAME).bin

# texane/stlink
flash:
	st-flash write $(NAME).bin 0x8000000

size:
	arm-none-eabi-size $(NAME).elf
debug:
	gnome-terminal --tab --command "arm-none-eabi-gdb -ex \"target remote localhost:3333\" \
		-ex \"set remote hardware-breakpoint-limit 5\" \
		-ex \"set remote hardware-watchpoint-limit 3\" main.elf"

halt:
	./stm32_reset.pl main.bin

open:
	openocd -f stlink-v2.cfg -f stm32f0x.cfg
