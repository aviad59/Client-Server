#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Synchronization.lib")
#define PORT_NUMBER 80
#define DEFAULT_SIZE 20

struct message {
	char* text; 
	int length_msg;
	int printed;
	int ClientID;
};

// Global variables
static int* LOCK = -1; // -1 used if lock wasnt used, 0 if clear, any ID number if taken
static struct message msg;

/*
___________________________Todo_List__________________________________
	Todo: Implement the "lock" solution ------ DONE!
	Todo: Add dynamic msg size for better input -------- DONE!
	Todo: Add a global msg length variable --------- DONE!
	Todo: Echo all clients the msg (IDEA: use timeout fo recv) ---- DONE!
	Todo: Implement overlapped mechanism
*/


/***************************************
 Function to update the msg variable 
 char by char.
****************************************/
int updataMSG(char chunk, int i) 
{
	if (msg.text == NULL || msg.text == "")
		msg.text = calloc(20, sizeof(char));

	if (i >= msg.length_msg - 1)
	{
		char* tmp;
		tmp = &msg.text[0];
		msg.text = NULL;
		msg.text = realloc(tmp, sizeof(char) * (i + 1));
		msg.length_msg = i;
		if (msg.text == NULL)
		{
			printf("There was a problem to reallc memorie\n");
			free(msg.text);
			return 1;
		}
	}
	msg.text[i] = chunk;
	return 0;
}
int sendMSG(SOCKET Client)
{
	if (msg.text != NULL)
	{
		send(Client,msg.text, msg.length_msg * sizeof(char), 0);
	}
	return 0;
}
int reciveMSG(SOCKET Client, int client_NUM)
{
	int size = 0;
	char chunk[DEFAULT_SIZE];
	ZeroMemory(chunk, DEFAULT_SIZE);
	int msg_index = 0;

	size = recv(Client, chunk, sizeof(char), 0);
	if (size && WSAGetLastError() != WSAETIMEDOUT)
	{
		do {
			if (LOCK != client_NUM && LOCK != -1 && LOCK != 0) {
				char warning[] = "Waiting for the LOCK...\n";
				send(Client, warning, sizeof(warning), 0);
				while (LOCK != 0) {} //waiting for the lock
			}
		
			LOCK = client_NUM; //LOCK
			updataMSG(chunk[0], msg_index);
			msg_index++;
			recv(Client, chunk, sizeof(char), 0);

		} while (chunk[0] != '\n');
		updataMSG('\n', msg_index); //replace \r with \n
		LOCK = 0; //UNLOCK
		
		msg.ClientID = client_NUM;

		msg.length_msg = msg_index + 1;
		msg_index = 0;
		printf(" Client %d said:\t %.*s \n", GetCurrentThreadId(), msg.length_msg, msg);
	}
	return size;
}

/*****************************************
  Function to handle each client
  represented by a thread.
******************************************/
int WINAPI clientHandler(void *data)
{
	SOCKET* client = (SOCKET*)data;
	SOCKET Client = *client;

	DWORD timeout_time = 1 * 1000;
	int local_printed = NULL;
	char *local_msg = NULL;

	char chunk[DEFAULT_SIZE];
	ZeroMemory(chunk, DEFAULT_SIZE);
	int client_NUM = GetCurrentThreadId();

	printf("\n----------------------------------------------\n");
	printf("Successfully connected to client %d.\n", client_NUM); //print new connection
	printf("----------------------------------------------\n");

	
	setsockopt(Client, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout_time, sizeof(timeout_time)); //set timeout 


	while(1)
	{
		if (local_printed != msg.printed)
		{
			sendMSG(Client);
			local_printed = msg.printed;
		}

		int size = reciveMSG(Client, client_NUM);
		if (size > 0) {
			msg.printed++;
			local_msg = msg.text;
		}
	}
	return 0;
}


/*****************************************
    Function to print project name.
******************************************/
void openMSG()
{
	printf("--------------------------------------------------------------------\n");
	printf(" _____                                          _ _            _   \n");
	printf("/  ___|                                        | (_)          | |  \n");
	printf("\\ `--.  ___ _ ____   _____ _ __   ______    ___| |_  ___ _ __ | |_ \n");
	printf(" `--. \\/ _ \\ '__\\ \\ / / _ \\ '__| |______|  / __| | |/ _ \\ '_ \\| __|\n");
	printf("/\\__/ /  __/ |   \\ V /  __/ |             | (__| | |  __/ | | | |_ \n");
	printf("\\____/ \\___|_|    \\_/ \\___|_|              \\___|_|_|\\___|_| |_|\\__|\n");
	printf("--------------------------------------------------------------------\n");
	printf("\n");
}


/*****************************************
			Main function.
******************************************/
int main()
{
/*
-----------------------------------
		StartUp Msg
-----------------------------------
*/
	openMSG();

/*
-----------------------------------
		Set Global Variables
-----------------------------------
*/
	msg.text = NULL;
	msg.length_msg = DEFAULT_SIZE;
	msg.printed = 0;
/*
-----------------------------------
		Initialize winsock
-----------------------------------
*/
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		printf("There was a problem initialize winsock");
		return 1;
	}
	printf("Successfully initialized winsock startup...\n");

/*
-----------------------------------
		Create socket
-----------------------------------
*/
	SOCKET listenting_sock;
	if ((listenting_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		printf("Could not create socket");
		return 1;
	}
	printf("Socket created successfully...\n");

	//bind the port
	struct sockaddr_in server;
	server.sin_addr.S_un.S_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT_NUMBER);
	if (bind(listenting_sock, (SOCKADDR*)&server,sizeof(server)) == SOCKET_ERROR)
	{
		printf("bind failed with error: %d", WSAGetLastError());
		return 1;
	}
	printf("Socket bend! now listening...\n");

	//listen 
	listen(listenting_sock, SOMAXCONN);

/*
----------------------------------
		Wait for connection
----------------------------------	
*/
	struct sockaddr_in client;
	int clientSize = sizeof(client);
	printf("Ready!!!\n");
	SOCKET soc_client;
	while (soc_client = accept(listenting_sock, 0,0))
	{
		if (soc_client == INVALID_SOCKET)
		{
			printf("Error accepting client");
			continue;
		}
		printf("Found new client! connecting...\n");
		HANDLE thread = CreateThread(NULL, 0, clientHandler, (void*)&soc_client, 0, NULL); 
	}

	printf("Finish.");
	closesocket(soc_client);
	WSACleanup();
	return 0;
}


