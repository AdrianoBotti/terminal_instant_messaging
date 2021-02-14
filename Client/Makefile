CC = gcc
CFLAGS = -Wall
SRCDIR = src
OBJS = $(SRCDIR)/msg_client.o $(SRCDIR)/msg_client_util.o $(SRCDIR)/requestReg.o $(SRCDIR)/requestDereg.o $(SRCDIR)/requestWho.o $(SRCDIR)/requestSend.o $(SRCDIR)/requestQuit.o

client: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	rm client $(SRCDIR)/*.o
