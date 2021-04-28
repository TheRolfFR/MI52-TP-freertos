
#name of the target
PROJECT = projet


#----------------------------------------------------------------------
# TOOL DEFINITIONS
#----------------------------------------------------------------------
PREFIX=arm-none-eabi-#arm-atollic-eabi-
AS      = $(PREFIX)as
CC      = $(PREFIX)gcc
CPP     = $(PREFIX)g++
LD      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE 	= $(PREFIX)size

#----------------------------------------------------------------------
# sources files
#----------------------------------------------------------------------
# main files
CSRCS = Src/main.c Src/syscalls.c Src/sysmem.c Src/system_stm32f4xx.c
CSRCS += Src/usart.c 
# add startup file to build
ASRCS = Startup/startup_stm32f401xe.s 

##############   FreeRTOS    ###############
#FreeRTOS sources directory
FREERTOS=./FreeRTOS

# add sources files for freeRTOS (depend on FreeRTOSConfig.h)
# others sources files can be added if you want to use 
# some OS services (timer, event, ..)
CSRCS += $(FREERTOS)/Source/queue.c
CSRCS += $(FREERTOS)/Source/tasks.c
CSRCS += $(FREERTOS)/Source/list.c
CSRCS += $(FREERTOS)/Source/portable/GCC/ARM_CM4F/port.c
#Choose a heap management sources :
CSRCS += $(FREERTOS)/Source/portable/MemMang/heap_4.c

#----------------------------------------------------------------------
# headers files
#----------------------------------------------------------------------
#PATH to STM32Cube library (here integrated in the project for headers)
STM_LIBRARY = .

INCLUDE_PATHS = -I. -I./Inc 

#CMSIs library 
INCLUDE_PATHS += -I$(STM_LIBRARY)/Drivers/CMSIS/Include \
					-I$(STM_LIBRARY)/Drivers/CMSIS/Device/ST/STM32F4xx/Include

##############   FreeRTOS    ###############
#add FreeRTOS Headers in the libraries search for gcc
INCLUDE_PATHS += -I$(FREERTOS)/Source/portable/GCC/ARM_CM4F 
INCLUDE_PATHS += -I$(FREERTOS)/Source/include 

#----------------------------------------------------------------------
#link script
#----------------------------------------------------------------------

LINKER_SCRIPT = ./STM32F401RETX_FLASH.ld

#----------------------------------------------------------------------
# COMPILER AND ASSEMBLER OPTIONS
#----------------------------------------------------------------------
#compiler options
CPU = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -specs=nano.specs
CC_FLAGS  = $(CPU) -c
AS_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16


#debug flags 
  CC_FLAGS +=  -O0 -g
  AS_FLAGS += -gdwarf-2
  CC_SYMBOLS += -DDEBUG -DUSE_FULL_ASSERT
#no debug
# CC_FLAGS +=  -Os


#define the target version (needed in some include file) :
CC_SYMBOLS = -DSTM32F401xE 

#----------------------------------------------------------------------
# linker options and library to link
#----------------------------------------------------------------------

LD_FLAGS = $(CPU) -Wl,--gc-sections -Wl,-Map=$(PROJECT).map,--cref 
LD_SYS_LIBS = -lm -lc -lgcc -lnosys 
LD_SYS_LIBS += -lstdc++ -lsupc++ 


#first target to avoid writting 'make all' each time
$(PROJECT):$(PROJECT).elf
#----------------------------------------------------------------------
# BUILD OBJECTS
#----------------------------------------------------------------------

# list all object files
OBJECTS = $(CSRCS:.c=.o) $(ASRCS:.s=.o) $(CPPSRCS:.cpp=.o) 
OBJECTS_LIB = $(CSRCS_LIB:.c=.o) 
dep = $(OBJECTS:.o=.d)  # one dependency file for each source

#compile all object files
%.o:%.c
	$(CC) $(CC_FLAGS) $(CC_SYMBOLS) $(INCLUDE_PATHS) -o $@ $<
%.o:%.s
	$(AS) $(AS_FLAGS) -o $@ $<
%.o:%.cpp
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98  $(INCLUDE_PATHS) -o $@ $< 

#dependencie
-include $(dep)   # include all dep files in the makefile
#dependencies files .d
%.d:%.c
	@$(CC) $(CFLAGS) $(INCLUDE_PATHS) $(CC_SYMBOLS) $< -MM -MT $(@:.d=.o) >$@

#----------------------------------------------------------------------
# BUILD PROJECT (link)
#----------------------------------------------------------------------

$(PROJECT).elf: $(OBJECTS) $(OBJECTS_LIB)
	$(LD) -o $@ $^ $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LD_SYS_LIBS) $(LIBS) $(LIBRARY_PATHS)
	$(SIZE) $@

# other binary files il needeed

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	@$(OBJCOPY) -O ihex $< $@

$(PROJECT).lst: $(PROJECT).elf
	@$(OBJDUMP) -Sdh $< > $@

lst: $(PROJECT).lst

size:
	$(SIZE) $(PROJECT).elf


#inclusion du contenu des fichiers .d
#-include $(CDEP)

all: $(PROJECT).bin $(PROJECT).hex 

.PHONY: clean
clean:
	rm -f $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst $(OBJECTS) $(OBJECTS_LIB) $(dep)

.PHONY: cleandep
cleandep:
	rm -f $(dep)



