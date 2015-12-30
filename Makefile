


all: main.c
	$(CC) -o watchpuppy main.c

cross: main.c
	$(CC) -o watchpuppy main.c $(CFLAGS) $(LDFLAGS)
