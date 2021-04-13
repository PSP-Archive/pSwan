BINARY = out
INCLUDE0 = oswan/source
INCLUDE1 = include
INCLUDE2 = csdlibrary/include
INCLUDE3 = csdlibrary/api-include
LIBRARY1 = mylib
LIBRARY2 = csdlibrary/lib
OBJECTS = psp/startup.o psp/main.o\
          psp/pg.o psp/filer.o psp/menu.o psp/sound.o\
          oswan/source/2xSaI.o oswan/source/audio.o oswan/source/gpu.o oswan/source/io.o oswan/source/log.o oswan/source/memory.o oswan/source/rom.o oswan/source/ws.o\
          oswan/source/nec/nec.o oswan/source/seal/audiow32.o

all: $(BINARY)

$(BINARY): $(OBJECTS)
	ee-ld -s -O0 $(OBJECTS) -M -Ttext 8900000 -q -L$(LIBRARY1) -L$(LIBRARY2) -lcsd -l_c -lmylib -o $@ > pswan.map
	outpatch
	elf2pbp outp "pSwan v0.07" ICON0.PNG
	cp EBOOT.PBP C:\PS2Dev\PSPE\ms0\PSP\GAME\WSC
	cp EBOOT.PBP J:\PSP\GAME\PSWAN

%.o : %.c
	ee-gcc -march=r4000 -O3 -I$(INCLUDE0) -I$(INCLUDE1) -I$(INCLUDE2) -I$(INCLUDE3) -fomit-frame-pointer -g -mgp32 -mlong32 -c $< -o $@

%.o : %.s
	ee-gcc -march=r4000 -g -mgp32 -c -xassembler -O -o $@ $<

clean:
	del /s /f *.o *.map out outp



#OBJECTS = startup.o main.o\
#	source/2xSaI.o source/audio.o source/gpu.o source/io.o source/log.o source/memory.o source/rom.o source/ws.o\
