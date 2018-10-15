## This is a project made within Seoul National University of Science and Technology
## Embedded Systems Laboratory 2015 - Raimarius Tolentino Delgado

#######################################################################################################
XENOMAI_PATH=/rtProj/xenomai

CFLAGS_OPTIONS = -Wall -Wno-unused-but-set-variable -Wno-unused-variable 

INC_XENO = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin posix --cflags)
LIB_XENO = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin posix --ldflags)

INC_DIRS = $(INC_XENO)

CFLAGS   = $(CFLAGS_OPTIONS) $(INC_DIRS) -I../drivers/led_drv/
LDFLAGS	 = $(LIB_XENO) -lm -ludev -I./

SOURCES	=	main.cpp \
			xeno_posix_task.cpp \
			xeno_xddp.cpp 
			 
CUR_DIR = ./
OBJ_DIR = obj
OUT_DIR = bin
EXEC_TARGET	= m4k_controller
START	= start

ifeq ($(wildcard main.cpp),)
CC = $(CROSS_COMPILE)gcc
else
CC = $(CROSS_COMPILE)g++
endif

LD = $(CROSS_COMPILE)ld
AS = $(CROSS_COMPILE)as

CHMOD	= /bin/chmod
MKDIR	= /bin/mkdir
ECHO	= echo
RM	= /bin/rm
#######################################################################################################
OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SOURCES)))))
#######################################################################################################
vpath %.c  $(CUR_DIR) $(INC_SERVO) $(INC_EMBD)
vpath %.cpp $(CUR_DIR) $(INC_SERVO) $(INC_EMBD)
#######################################################################################################

ifeq ($(wildcard $(START).sh),)
all: 	$(OUT_DIR)/$(EXEC_TARGET) $(START)
	@$(ECHO) BUILD DONE.
	@$(CHMOD) +x $(START).sh
else
all: 
	@$(ECHO) BUILD ERROR: Run make clean first!
endif

$(START): 
	@printf "## This is a project made within Seoul National University of Science and Technology \n" > $(START).sh
	@printf "## Embedded Systems Laboratory 2015 - Raimarius Tolentino Delgado \n\n" >> $(START).sh
	@printf "## Start-up for dynamically linked executable file \n\n\n\n" >> $(START).sh
	@printf "export LD_LIBRARY_PATH=$(XENOMAI_PATH)/lib:$(LIB_DIR):$(LIB_URG) \n" >> $(START).sh
	@printf "./$(OUT_DIR)/$(EXEC_TARGET)\n" >> $(START).sh

$(OUT_DIR)/$(EXEC_TARGET): $(OBJECTS)
	@$(MKDIR) -p $(OUT_DIR); pwd > /dev/null
	$(CC) -o $(OUT_DIR)/$(EXEC_TARGET) $(OBJECTS) $(LDFLAGS)

$(OBJ_DIR)/%.o : %.cpp
	@$(MKDIR) -p $(OBJ_DIR); pwd > /dev/null
	$(CC) -MD $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o : %.c
	@$(MKDIR) -p $(OBJ_DIR); pwd > /dev/null
	$(CC) -MD $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -rf \
		$(OBJ_DIR)/* \
		$(OBJ_DIR)   \
			$(OUT_DIR)/* \
			$(OUT_DIR)   \
			*.dat	     \
		$(START)*
re:
	make clean
	make -j4


.PHONY: all clean 
#######################################################################################################
# Include header file dependencies generated by -MD option:
-include $(OBJ_DIR_CUR)/*.d


