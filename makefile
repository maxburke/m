CC = clang
CFLAGS = -g -std=c89 -Wno-variadic-macros -Wall -Wextra -Werror -pedantic -I/usr/local/include -D_FILE_OFFSET_BITS=64 -DM_LITTLE_ENDIAN=1
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o : %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

m : $(OBJS)
	clang -o $@ $(LIBS) $^ $(LDFLAGS)

.PHONY : clean
clean :
	rm *.o
	rm m

.PHONY : all
all: m

