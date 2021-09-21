all: 
	g++ ./servidor/servidorTCP.cpp -lpthread -o servidorTCP
	g++ ./cliente/clienteTCP.cpp -lpthread -o clienteTCP
