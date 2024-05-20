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
	ifdef SYSTEMD
		$(INSTALL) -D $(TARGET) /usr/sbin/$(TARGET)
		cp daemon/systemd/rootkit-server.service /etc/systemd/system/
		systemctl daemon-reload
		systemctl enable rootkit-server
		ifdef OPENRC
		$(INSTALL) -D $(TARGET) /usr/sbin/$(TARGET)
		cp daemon/openrc/rootkit-server /etc/init.d/
	rc-update add rootkit-server default
		else ifdef INITRC
		$(INSTALL) -D $(TARGET) /usr/sbin/$(TARGET)
		cp daemon/initrc/rootkit-server /etc/init.d/
		update-rc.d rootkit-server defaults
	endif

uninstall:
	rm -f /usr/sbin/$(TARGET)
	ifdef SYSTEMD
		systemctl disable rootkit-server
		rm -f /etc/systemd/system/rootkit-server.service
		systemctl daemon-reload
	else ifdef OPENRC
		rc-update del rootkit-server default
		rm -f /etc/init.d/rootkit-server
	else ifdef INITRC
		update-rc.d -f rootkit-server remove
		rm -f /etc/init.d/rootkit-server
	endif
