.PHONY: all serverA serverB mainserver tester

all:

	g++ -g serverA.cpp -o serverA
	g++ -g serverB.cpp -o serverB
	g++ -g servermain.cpp -o mainserver
	g++ -g client.cpp -o client 
	g++ -g ServerTester.cpp -o tester

serverA:

	./serverA

serverB:

	./serverB

mainserver:

	./mainserver

tester:

	./tester