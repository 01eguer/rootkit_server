TARGET   := rootkit-server
OBJ_DIR  := obj
SRC_DIR  := src
CC       := gcc
CFLAGS   := -O2 -Wall -pthread -lcrypto 
# CFLAGS   := -Wall -pthread -lcrypto 

MKDIR    := mkdir -p
INSTALL  := install

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
	$(CC) -c -o $@ $< $(CFLAGS)

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
	@if [ "$(SYSTEMD)" = "1" ]; then \
		$(INSTALL) -D $(TARGET) /usr/sbin/$(TARGET); \
		cp src/daemon/systemd/rootkit-server.service /etc/systemd/system/; \
		systemctl daemon-reload; \
		#systemctl enable rootkit-server; \
	fi
	@if [ "$(OPENRC)" = "1" ]; then \
		$(INSTALL) -D $(TARGET) /usr/sbin/$(TARGET); \
		cp src/daemon/openrc/rootkit-server /etc/init.d/; \
		#rc-update add rootkit-server default; \
	fi
	@if [ "$(INITRC)" = "1" ]; then \
		$(INSTALL) -D $(TARGET) /usr/sbin/$(TARGET); \
		cp src/daemon/initrc/rootkit-server /etc/init.d/; \
		#update-rc.d rootkit-server defaults; \
	fi

uninstall:
	@if [ "$(SYSTEMD)" = "1" ]; then \
		systemctl disable rootkit-server; \
		rm -f /etc/systemd/system/rootkit-server.service; \
		systemctl daemon-reload; \
	fi
	@if [ "$(OPENRC)" = "1" ]; then \
		rc-update del rootkit-server default; \
		rm -f /etc/init.d/rootkit-server; \
	fi
	@if [ "$(INITRC)" = "1" ]; then \
		update-rc.d -f rootkit-server remove; \
		rm -f /etc/init.d/rootkit-server; \
	fi
	rm -f /usr/sbin/$(TARGET)

