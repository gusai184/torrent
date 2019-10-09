all:torrent

torrent: tracker.o commands.o
	g++ tracker.o commands.o -o  torrent

tracker.o: tracker.cpp
	g++ -c tracker.cpp

command.o: commands.cpp
	g++ -c commands.cpp

r:
	rm tracker.o commands.o 
