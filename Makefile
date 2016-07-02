# compilation options
CC=avr-gcc
OBJCOPY=avr-objcopy
CMCU=atmega644p
CFLAGS=-Os -std=gnu99 -mmcu=$(CMCU)

main.hex : main.out
	$(OBJCOPY) -j .text -j .data -O ihex main.out main.hex

main.out : main.o
	$(CC) $(CFLAGS) -o main.out -Wl,-Map,main.map main.o

main.o : main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o *.out *.map *.hex
