#include "TCPConnect.h"

void TCPConnect::connectt(TCPConnection *t, SOCKET *connectSocket, SOCKET *listenSocket, char* ipAddress, int port, char *request, char *response, bool closeConnection, unsigned long int nonBlockingMode, Buffer *b) {
	int iResult = -1;
	iResult = createSocket(t, connectSocket, listenSocket, ipAddress, port, nonBlockingMode);
	std::cout << "\nLISTEN SOCKET IZ SERVERA: %d\n" << *connectSocket << std::endl;

	if (iResult != 0) {
		//WSACleanup();

		// ERROR
	}

}

int TCPConnect::createSocket(TCPConnection *t, SOCKET *connectSocket, SOCKET *listenSocket, char* ipAddress, int port, char *request, char *response, bool closeConnection, unsigned long int nonBlockingMode, Buffer *b)
{
	if (listenSocket != nullptr) {
		char cport[16];
		itoa(port, cport, 10);
		int iResult = -1;
		iResult = listenSocketFunc(listenSocket, cport);
		if (iResult == 1) {
			// Ako je ovde greska, kraj rada
			return 1;
		}
		while (!exit)
		{
			printf("\nLISTEN SOCKET IZ SERVERA: %d\n", listenSocket);
			iResult = ioctlsocket(*connectSocket, FIONBIO, &nonBlockingMode);
			selectt(listenSocket, 0, 0);
			// Wait for clients and accept client connections.
			// Returning value is acceptedSocket used for further
			// Client<->Server communication. This version of
			// server will handle only one client.
			iResult = acceptt(connectSocket, listenSocket);
			if (iResult == 1) {
				// Ako je ovde greska, kraj rada
				return 1;
			}

			ChangeState(t, TCPReceive::Instance());
			t->receive(connectSocket, listenSocket, ipAddress, port, request, response, closeConnection, nonBlockingMode, b);

		}
	}
	else {
		// create a socket
		*connectSocket = socket(AF_INET,
			SOCK_STREAM,
			IPPROTO_TCP);

		if (*connectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			//WSACleanup();
			return 1; //fail!
		}

		// create and initialize address structure
		sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = inet_addr(ip);
		serverAddress.sin_port = htons(port);
		// connect to server specified in serverAddress and socket connectSocket
		if (connect(*connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
		{
			printf("Unable to connect to server.\n");
			closesocket(*connectSocket);
			//WSACleanup();
			return 1;
		}

		ChangeState(t, TCPSend::Instance());
		t->send(connectSocket, listenSocket, ipAddress, port, request, response, closeConnection, nonBlockingMode, b);
	}


	return 0;
}



int TCPConnect::listenSocketFunc(SOCKET * listenSocket, char * port)
{

	// variable used to store function return value
	int iResult;

	// Prepare address information structures
	addrinfo *resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

									 // Resolve the server address and port
	iResult = getaddrinfo(NULL, port, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	*listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (*listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(*listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(*listenSocket);
		WSACleanup();
		return 1;
	}

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(*listenSocket, FIONBIO, &nonBlockingMode);

	// Set listenSocket in listening mode
	iResult = listen(*listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(*listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server initialized, waiting for clients.\n");

	return iResult;
}

int TCPConnect::selectt(SOCKET * socket, int type, int *exit)
{
	FD_SET set;
	timeval timeVal;
	int iResult;

	do {
		iResult = 0;
		FD_ZERO(&set);
		// Add socket we will wait to read from
		FD_SET(*socket, &set);

		// Set timeouts to zero since we want select to return
		// instantaneously
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;
		if (type == 0)  //receive
		{
			iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		}
		else {
			iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);
		}
		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			return -1; //error code: -1
		}

		// now, lets check if there are any sockets ready
		if (iResult == 0)
		{
			//printf("\n SPAVAM! ");
			// there are no ready sockets, sleep for a while and check again
			Sleep(200);
		}

	} while (iResult == 0 && *exit == 0);

	return iResult;
}

int TCPConnect::acceptt(SOCKET * acceptedSocket, SOCKET* listenSocket)
{
	*acceptedSocket = accept(*listenSocket, NULL, NULL);

	if (*acceptedSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(*listenSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}