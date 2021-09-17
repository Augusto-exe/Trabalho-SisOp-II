all: 
	gcc servidorTCP.cpp -lpthread -o servidorTCP && gcc clienteTCP.cpp -lpthread -o clienteTCP