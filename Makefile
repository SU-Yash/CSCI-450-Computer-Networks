.PHONY: all serverA serverB mainserver

all:

	g++ -g serverA.cpp -o serverA
	g++ -g serverB.cpp -o serverB
	g++ -g servermain.cpp -o mainserver
	g++ -g client.cpp -o client 

serverA:

	./serverA

serverB:

	./serverB

mainserver:

	./mainserver
