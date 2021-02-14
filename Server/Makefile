CC = gcc
CFLAGS = -Wall
SRCDIR = src
OBJS = $(SRCDIR)/msg_server.o $(SRCDIR)/msg_server_util.o $(SRCDIR)/register.o $(SRCDIR)/deregister.o $(SRCDIR)/who.o $(SRCDIR)/send.o $(SRCDIR)/quit.o

server: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	rm server $(SRCDIR)/*.o
