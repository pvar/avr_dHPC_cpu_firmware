### ---------------------------------------------------------------------------
### EXECUTABLES
### ---------------------------------------------------------------------------

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size

### ---------------------------------------------------------------------------
### FILES
### ---------------------------------------------------------------------------

SOURCES = $(wildcard *.c)
HEADERS = $(SOURCES:.c=.h)
OBJECTS = $(SOURCES:.c=.o)
LISTINGS = $(SOURCES:.c=.lst)
#INCPATH = /usr/lib/avr/include
#LIBRARIES = $(INCPATH)/stdio.h $(INCPATH)/stdlib.h $(INCPATH)/inttypes.h

### ---------------------------------------------------------------------------
### FLAGS
### ---------------------------------------------------------------------------

DEVICE = atmega644p
CLOCK = 20000000UL
BAUD = 57600
TUNNING = -Os -fshort-enums
STANDARD = -std=gnu99
WARNINGS = -Wall -Wstrict-prototypes
CCFLAGS =   $(TUNNING)  \
            $(STANDARD) \
            $(WARNINGS) \
	        -mmcu=$(DEVICE)  \
	        -DF_CPU=$(CLOCK) \
	        -DBAUD=$(BAUD)

LDFLAGS = -Wl,-Map,main.map 
LDFLAGS += -Wl,--gc-sections 

### ---------------------------------------------------------------------------
### TARGETS
### ---------------------------------------------------------------------------

hex: main.elf main.eeprom
	$(OBJCOPY) -j .text -j .data -O ihex main.elf main.hex
	$(AVRSIZE) --format=avr --mcu=$(DEVICE) main.elf

asm: $(LISTINGS)

clean:
	-rm -f *.o *.elf *.map *.lst *.eeprom *~

rebuild: clean hex

### ---------------------------------------------------------------------------

%.o: %.c $(HEADERS)
	 $(CC) $(CCFLAGS) -c -o $@ $<

%.lst: %.o
	$(OBJDUMP) -S --disassemble $< > $@
	mv *.lst asm/

main.elf: $(OBJECTS)
	$(CC) $(CCFLAGS) $(LDFLAGS) $^ -o $@

main.eeprom: main.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex main.elf main.eeprom

