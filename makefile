all: 
	g++ ./servidor/sessionManager.cpp ./servidor/servidorTCP.cpp -lpthread -o servidorTCP
	g++ -o clienteTCP ./cliente/interface.cpp ./cliente/clienteTCP.cpp -lpthread
