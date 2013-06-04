NAME=libslowdown
VERSION=0.1

TARGET=$(NAME).so.$(VERSION)
OBJS=$(NAME).o
CC=gcc
CFLAGS= -fPIC -Wall -O2 -g
SHAREDFLAGS= -nostartfiles -shared -Wl,-soname,$(NAME).so.0

ifeq ($(DEBUG),1)
CFLAGS += -DDEBUG
endif

all: $(TARGET)

%.o: %.c
	$(CC) -I. $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(SHAREDFLAGS) $(OBJS) -o $(TARGET) -lc -ldl

clean:
	rm -f $(OBJS) $(TARGET)
