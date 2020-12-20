//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <windows.h>
//
//#pragma comment(lib, "Ws2_32.lib")
//#pragma comment(lib, "Synchronization.lib")
//#define PORT_NUMBER 80
//#define DEFAULT_SIZE 20
//
// Global variables
//static int* LOCK = -1; // -1 used if lock wasnt used, 0 if clear, any ID number if taken
//static int b_Stop = 1;
//DWORD NumberOfEvent = 0;
//HANDLE hConnectEvent;
//
//static WSABUF msg;
//
///*
//___________________________Todo_List__________________________________
//	Todo: Implement overlapped mechanism ------ DONE!
//	Todo: Update the interface
//*/
//
//
///***************************************
// Function to update the msg variable
// char by char.
//****************************************/
//int updataMSG(char chunk, int i)
//{
//	if (msg.buf == NULL || msg.buf == "")
//		msg.buf = calloc(20, sizeof(char));
//
//	if (i <= msg.len)
//	{
//		char* tmp;
//		tmp = &msg.buf[0];
//		msg.buf = NULL;
//		msg.buf = realloc(tmp, sizeof(char) * (i + 1));
//		msg.len = i;
//		if (msg.buf == NULL)
//		{
//			printf("There was a problem to reallc memorie\n");
//			free(msg.buf);
//			return 1;
//		}
//	}
//	msg.buf[i] = chunk;
//	return 0;
//}
//int sendMSG(SOCKET Client, WSAOVERLAPPED overlapped)
//{
//	DWORD BytesSend = 0;
//	DWORD BytesSize = 0;
//	if (msg.buf != NULL)
//	{
//		WSASend(Client, &msg, &BytesSize, &BytesSend, 0, &overlapped, 0);
//	}
//	return 0;
//}
//int reciveMSG(SOCKET Client, int client_NUM, WSAOVERLAPPED overlapped)
//{
//	int size = 0;
//	char* chunk[DEFAULT_SIZE];
//	ZeroMemory(chunk, DEFAULT_SIZE);
//	int msg_index = 0;
//	DWORD RecvBytes = 0;
//	DWORD Flags = 0;
//
//	size = WSARecv(Client, &chunk, 1, &RecvBytes, &Flags, &overlapped, NULL);
//	if (size != -1 && WSAGetLastError() != WSA_IO_PENDING)
//	{
//		do {
//			if (LOCK != client_NUM && LOCK != -1 && LOCK != 0) {
//				char warning[] = "Waiting for the LOCK...\n";
//				send(Client, warning, sizeof(warning), 0);
//				while (LOCK != 0) {} //waiting for the lock
//			}
//
//			LOCK = client_NUM; //LOCK
//			updataMSG(chunk[0], msg_index);
//			msg_index++;
//			WSARecv(Client, &chunk, 1, &RecvBytes, &Flags, &overlapped, NULL);
//
//		} while (chunk[0] != '\0' && chunk[0] != '\n');
//		LOCK = 0; //UNLOCK
//
//		msg.len = msg_index + 1;
//		msg_index = 0;
//		printf(" Client %d said:\t %.*s \n", client_NUM, msg.len, msg);
//	}
//	return size;
//}
///*****************************************
//  Function to handle each client
//  represented by a thread.
//******************************************/
//int WINAPI clientHandler(SOCKET Client, int client_NUM, WSAOVERLAPPED overlapped)
//{
//	int local_printed = NULL;
//	char* local_msg = NULL;
//
//	char chunk[DEFAULT_SIZE];
//	ZeroMemory(chunk, DEFAULT_SIZE);
//
//	DWORD RecvBytes = 0;
//	DWORD Flags = 0;
//
//	printf("\n----------------------------------------------\n");
//	printf("Successfully connected to client %d.\n", client_NUM); //print new connection
//	printf("----------------------------------------------\n");
//
//	while (1) {
//		reciveMSG(Client, ++client_NUM, overlapped);
//		WSARecv(Client, &msg, 1, &RecvBytes, &Flags, &overlapped, NULL);
//		sendMSG(Client, overlapped);
//	}
//
//}
///*****************************************
//	Function to print project name.
//******************************************/
//void openMSG()
//{
//	printf("--------------------------------------------------------------------\n");
//	printf(" _____                                          _ _            _   \n");
//	printf("/  ___|                                        | (_)          | |  \n");
//	printf("\\ `--.  ___ _ ____   _____ _ __   ______    ___| |_  ___ _ __ | |_ \n");
//	printf(" `--. \\/ _ \\ '__\\ \\ / / _ \\ '__| |______|  / __| | |/ _ \\ '_ \\| __|\n");
//	printf("/\\__/ /  __/ |   \\ V /  __/ |             | (__| | |  __/ | | | |_ \n");
//	printf("\\____/ \\___|_|    \\_/ \\___|_|              \\___|_|_|\\___|_| |_|\\__|\n");
//	printf("--------------------------------------------------------------------\n");
//	printf("\n");
//}
///*****************************************
//			Main function.
//******************************************/
//int main()
//{
//	/*
//	-----------------------------------
//			StartUp Msg
//	-----------------------------------
//	*/
//	openMSG();
//
//	/*
//	-----------------------------------
//			Set Global Variables
//	-----------------------------------
//	*/
//	msg.buf = NULL;
//	msg.len = DEFAULT_SIZE;
//
//	/*
//	-----------------------------------
//			Initialize winsock
//	-----------------------------------
//	*/
//	WSADATA data;
//	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
//	{
//		printf("There was a problem initialize winsock");
//		return 1;
//	}
//	printf("Successfully initialized winsock startup...\n");
//
//	/*
//	-----------------------------------
//			Create socket
//	-----------------------------------
//	*/
//	SOCKET listenting_sock;
//	if ((listenting_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
//	{
//		printf("Could not create socket");
//		return 1;
//	}
//	printf("Socket created successfully...\n");
//
//	//bind the port
//	struct sockaddr_in server;
//	server.sin_addr.S_un.S_addr = INADDR_ANY;
//	server.sin_family = AF_INET;
//	server.sin_port = htons(PORT_NUMBER);
//	if (bind(listenting_sock, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR)
//	{
//		printf("bind failed with error: %d", WSAGetLastError());
//		return 1;
//	}
//	printf("Socket bend! now listening...\n");
//
//	//listen 
//	listen(listenting_sock, SOMAXCONN);
//
//	/*
//	----------------------------------
//			Accept connection
//	----------------------------------
//	*/
//	struct sockaddr_in client;
//	int clientSize = sizeof(client);
//	printf("Ready!!!\n");
//	SOCKET soc_client = accept(listenting_sock, 0, 0);
//
//	WSABUF DataBuf;
//	DWORD BytesTransferred;
//	DWORD Flags = 0;
//	DWORD index;
//	WSAOVERLAPPED AcceptOverlapped;
//	char buffer[DEFAULT_SIZE];
//
//
//	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
//	EventArray[NumberOfEvent] = WSACreateEvent();
//	ZeroMemory(&AcceptOverlapped, sizeof(WSAOVERLAPPED));
//
//	AcceptOverlapped.hEvent = EventArray[NumberOfEvent];
//	
//	do
//	{
//		index = WSAWaitForMultipleEvents(NumberOfEvent, EventArray, FALSE, WSA_INFINITE, FALSE);
//		WSAResetEvent(EventArray[index - WSA_WAIT_EVENT_0]);
//		WSAGetOverlappedResult(soc_client, &AcceptOverlapped, &BytesTransferred, FALSE, &Flags);
//
//		NumberOfEvent++;
//
//		clientHandler(soc_client, index, AcceptOverlapped);
//
//		Flags = 0;
//		ZeroMemory(&AcceptOverlapped, sizeof(WSAOVERLAPPED));
//		AcceptOverlapped.hEvent = EventArray[index - WSA_WAIT_EVENT_0];
//
//	} while (soc_client = accept(listenting_sock, 0, 0));
//}
//
//
