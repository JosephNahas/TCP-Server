#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")


using namespace std;

void main() {

	// initialize winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);

	if (wsOk != 0) {
		cerr << "Can't initialize winsock! Quitting" << endl;
		return;
	}

	// create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create socket! Quitting" << endl;
		return;
	}

	// bind socket to an ip address and port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	
	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// tell winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	fd_set master;
	FD_ZERO(&master);

	FD_SET(listening, &master);

	while (true) 
	{
		fd_set copy = master;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			sockaddr_in client;
			int clientSize = sizeof(client);

			if (sock == listening)
			{
				sockaddr_in client;
				int clientSize = sizeof(client);

				// accept a new connection
				SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

				// add the new connection to the list of connected clients
				FD_SET(clientSocket, &master);

				char host[NI_MAXHOST]; // client's remote name
				char service[NI_MAXSERV];// port the client is connected on

				ZeroMemory(host, NI_MAXHOST);
				ZeroMemory(service, NI_MAXSERV);

				inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
				cout << host << " has joined the room on port " << ntohs(client.sin_port) << endl;

				// send a welcome message to the connected client
				string welcomeMsg = "Welcome to server \n";
				send(clientSocket, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
				
			}

			else
			{
				char buf[4096];
				ZeroMemory(buf, 4096);


				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// drop client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{

					cout << string(buf, 0, bytesIn) << endl;
					// send message back
					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock == sock)
						{
							send(outSock, buf, bytesIn, 0);
						}
					}
				}
			}
		}
	}

	// shutdown winsock
	WSACleanup();
}


