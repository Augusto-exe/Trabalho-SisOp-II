all: 
	g++ servidorTCP.cpp -lpthread -o servidorTCP
	g++ clienteTCP.cpp -lpthread -o clienteTCP
