TARGET   := rootkit-server
OBJ_DIR  := obj
SRC_DIR  := src
CC       := gcc
CFLAGS   := -O2 -Wall -pthread -lcrypto 
# CFLAGS   := -Wall -pthread -lcrypto 

MKDIR    := mkdir -p

# REPLICATE FOLDER STRUCTURE
SRC_SUBDIRS	:= $(shell find $(SRC_DIR) -type d )
OBJ_SUBDIRS	:= $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(SRC_SUBDIRS))

# GET .C FILES AND FUTURE .O FILES
SRCS       := $(shell find $(SRC_DIR) -type f -name "*.c")
OBJS       := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(patsubst %.c,%.o,$(SRCS)))

.PHONY: dir files clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_SUBDIRS) 
	$(CC) $(CFLAGS) -c -o $@ $<

# $^ <- todas las dependencias de la regla
# $@ <- objetivo de la regla
# $< <- primera dependencia de la regla

dir:
	$(info $(SRC_SUBDIRS))
	$(info $(OBJ_SUBDIRS))

files:
	$(info $(SRCS))
	$(info $(OBJS))

$(OBJ_SUBDIRS):
	$(MKDIR) $(OBJ_SUBDIRS)

clean:
	rm -rf $(TARGET) $(OBJ_SUBDIRS)

install: $(TARGET)
	$(INSTALL) -D $(TARGET) /usr/sbin/$(TARGET)

uninstall:
	rm -f /usr/sbin/$(TARGET)
