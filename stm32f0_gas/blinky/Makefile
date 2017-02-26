program: assemble link binary

assemble:
	arm-none-eabi-as stm32f051_startup.s -mthumb -mcpu=cortex-m0 -g -o main.o

link:
	arm-none-eabi-ld -T stm32f051.ld -Map=main.map main.o -o main.elf

binary:
	arm-none-eabi-objcopy -O binary main.elf main.bin

flash:
	st-flash write main.bin 0x8000000

clean:
	rm -f *.elf *.map *.o *.bin
