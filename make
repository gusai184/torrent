
tracker:
	makefile -f makefile.tracker

client:
	makefile -f makefile.client

r:
	rm listener.o client.o tracker.o commands.o
