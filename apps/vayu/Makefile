# -*- Makefile -*-

#define path to Root directory
ROOT_DIR := ../..
#Defining the device / Part Number. used in compiler flags
DEVICE = EFR32BG13P732F512GM48
#defining flash size of the part. Used in chip-erase recipe of openocd
FLASH_SIZE_BYTES = 524288 
#define mcu type used in emlib dir/file paths 
EFR32_MCU = efr32bg13p
#EFR32 cpu series
EFR32_SERIES = efr32xg1x
#defin project name for the output files. change the project name as per the new project name
PROJECTNAME = vayu
#define board name. This will help include the board specific source and header files
BOARDNAME = vayu

#paths to header files of the project
#add new paths for the new include folders of the project here
PROJECT_INCLUDEPATHS += \
-I$(ROOT_DIR)/arch/platform/efr32/$(BOARDNAME)/ \
-I$(ROOT_DIR)/arch/platform/efr32/akashvani1/ \
-I$(ROOT_DIR)/apps/$(PROJECTNAME)/ \
-I$(ROOT_DIR)/tarang/dev/sensirion/ \
-I$(ROOT_DIR)/tarang/dev/sht4x/ \
-I$(ROOT_DIR)/tarang/dev/guart/ \
-I$(ROOT_DIR)/tarang/dev/ntc/ \

#paths to source files of the project. Add/remove project source files here
PROJECT_SRC_C_CXX += \
$(ROOT_DIR)/arch/platform/efr32/$(BOARDNAME)/board.c \
$(ROOT_DIR)/arch/platform/efr32/akashvani1/board-akashvani1.c \
$(ROOT_DIR)/apps/$(PROJECTNAME)/$(PROJECTNAME).c \
$(ROOT_DIR)/tarang/dev/sensirion/sensirion.c \
$(ROOT_DIR)/tarang/dev/sht4x/sht4x.c \
$(ROOT_DIR)/tarang/dev/guart/guart.c \
$(ROOT_DIR)/tarang/dev/ntc/ntc.c \

# Uncomment below flag to use SWO feature for debugging
#CFLAGS+ = -DUSE_SWO_DEBUG 
USE_FLOAT_DGB_IO = YES
-include $(ROOT_DIR)/arch/platform/efr32/Makefile.platform
