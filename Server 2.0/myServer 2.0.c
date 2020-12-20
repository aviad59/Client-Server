#include <winsock2.h>
#include <windows.h>
#include <stdio.h>


#define PORT 80
#define DATA_BUFSIZE 20
#pragma comment(lib, "ws2_32.lib")

typedef struct _SOCKET_INFORMATION {
    CHAR Buffer[DATA_BUFSIZE];
    WSABUF DataBuf;
    SOCKET Socket;
    WSAOVERLAPPED Overlapped;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

DWORD EventTotal = 0;
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION CriticalSection;


DWORD WINAPI HandleCommand(LPVOID lpParameter)
{

}

DWORD WINAPI ProcessIO(LPVOID lpParameter)

{
    DWORD Index;
    DWORD Flags;
    LPSOCKET_INFORMATION SI;
    DWORD BytesTransferred;
    DWORD i;
    DWORD RecvBytes, SendBytes;

    // Process asynchronous WSASend, WSARecv requests
    while (TRUE)
    {
        if ((Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMultipleEvents() failed %d\n", WSAGetLastError());
            return 0;
        }
        else
            printf("WSAWaitForMultipleEvents() is OK!\n");

        // If the event triggered was zero then a connection attempt was made
        // on our listening socket.
        if ((Index - WSA_WAIT_EVENT_0) == 0)
        {
            WSAResetEvent(EventArray[0]);
            continue;
        }
 

        SI = SocketArray[Index - WSA_WAIT_EVENT_0];
        WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

        if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred, FALSE, &Flags) == FALSE || BytesTransferred == 0)
        {
            printf("Closing socket %d\n", SI->Socket);
            if (closesocket(SI->Socket) == SOCKET_ERROR)
            {
                printf("closesocket() failed with error %d\n", WSAGetLastError());
            }
            else
                printf("closesocket() is OK!\n");



            GlobalFree(SI);
            WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

            // Cleanup SocketArray and EventArray by removing the socket event handle
            // and socket information structure if they are not at the end of the arrays
            EnterCriticalSection(&CriticalSection);


            if ((Index - WSA_WAIT_EVENT_0) + 1 != EventTotal)
                for (i = Index - WSA_WAIT_EVENT_0; i < EventTotal; i++)
                {
                    EventArray[i] = EventArray[i + 1];
                    SocketArray[i] = SocketArray[i + 1];
                }



            EventTotal--;

            LeaveCriticalSection(&CriticalSection);

            continue;

        }
        
        /*--------------
           recived data
        ----------------*/



        // Check to see if the BytesRECV field equals zero. If this is so, then
        // this means a WSARecv call just completed so update the BytesRECV field
        // with the BytesTransferred value from the completed WSARecv() call.
        if (SI->BytesRECV == 0)
        {
            SI->BytesRECV = BytesTransferred;
            SI->BytesSEND = 0;
        }
        else
        {
            SI->BytesSEND += BytesTransferred;
        }


        if (SI->BytesRECV > SI->BytesSEND)
        {
            //check if the input is a command
            if (SI->Buffer[0] == '$')
            {
                //refer to as a command
                HandleCommand(lpParameter);
            }


            // Post another WSASend() request.
            // Since WSASend() is not guaranteed to send all of the bytes requested,
            // continue posting WSASend() calls until all received bytes are sent
            ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
            SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
            SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
            SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

            if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0, &(SI->Overlapped), NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                {
                    printf("WSASend() failed with error %d\n", WSAGetLastError());
                    return 0;
                }
            }
            else
                printf("WSASend() is OK!\n");
        }
        else
        {
            SI->BytesRECV = 0;
            // Now that there are no more bytes to send post another WSARecv() request
            Flags = 0;
            ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
            SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
            SI->DataBuf.len = DATA_BUFSIZE;
            SI->DataBuf.buf = SI->Buffer;

            if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, &(SI->Overlapped), NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                {
                    printf("WSARecv() failed with error %d\n", WSAGetLastError());
                    return 0;
                }
            }
            else
                printf("WSARecv() is OK!\n");
        }
    }
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


int main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET listenting_sock, AcceptSocket;
    SOCKADDR_IN InternetAddr;
    DWORD Flags;
    DWORD ThreadId;
    DWORD RecvBytes;

    InitializeCriticalSection(&CriticalSection);

    /*
    -----------------------------------
            StartUp Msg
    -----------------------------------
    */
    openMSG();

    /*
    -----------------------------------
            Initialize winsock
    -----------------------------------
    */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
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
    if ((listenting_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket");
        return 1;
    }
    printf("Socket created successfully...\n");

    /*
    -----------------------------------
           Bind socket
    -----------------------------------
    */
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(PORT);

    if (bind(listenting_sock, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        printf("bind() failed with error %d\n", WSAGetLastError());
        return 1;
    }

    /*
    -----------------------------------
           Listen
    -----------------------------------
    */
    if (listen(listenting_sock, 5))
    {
        printf("listen() failed with error %d\n", WSAGetLastError());
        return 1;
    }


    // Setup the listening socket for connections
    if ((AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return 1;

    }


    if ((EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
        return 1;
    }
    else
        printf("WSACreateEvent() is OK!\n");

    // Create a thread to service overlapped requests
    if (CreateThread(NULL, 0, ProcessIO, NULL, 0, &ThreadId) == NULL)
    {
        printf("CreateThread() failed with error %d\n", GetLastError());
        return 1;
    }
    else
        printf("CreateThread() is OK!\n");

    EventTotal = 1;

    while (TRUE)
    {
        // Accept inbound connections
        if ((AcceptSocket = accept(listenting_sock, NULL, NULL)) == INVALID_SOCKET)
        {
            printf("accept() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        else
        {
            printf("\n-------------------------------\n");
            printf("\taccepted new client!");
            printf("\n-------------------------------\n\n");
        }

        EnterCriticalSection(&CriticalSection);

        // Create the socket information structure for the accepted socket
        if ((SocketArray[EventTotal] = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
        {
            printf("GlobalAlloc() failed with error %d\n", GetLastError());
            return 1;
        }


        // Fill in the details of our accepted socket
        SocketArray[EventTotal]->Socket = AcceptSocket;
        ZeroMemory(&(SocketArray[EventTotal]->Overlapped), sizeof(OVERLAPPED));
        SocketArray[EventTotal]->BytesSEND = 0;
        SocketArray[EventTotal]->BytesRECV = 0;
        SocketArray[EventTotal]->DataBuf.len = DATA_BUFSIZE;
        SocketArray[EventTotal]->DataBuf.buf = SocketArray[EventTotal]->Buffer;

        if ((SocketArray[EventTotal]->Overlapped.hEvent = EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT)
        {
            printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
            return 1;
        }

        // Post a WSARecv() request to to begin receiving data on the socket
        Flags = 0;

        if (WSARecv(SocketArray[EventTotal]->Socket,
            &(SocketArray[EventTotal]->DataBuf), 1, &RecvBytes, &Flags, &(SocketArray[EventTotal]->Overlapped), NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != ERROR_IO_PENDING)
            {
                printf("WSARecv() failed with error %d\n", WSAGetLastError());
                return 1;
            }
        }
        else

            printf("WSARecv() should be working!\n");
        EventTotal++;
        LeaveCriticalSection(&CriticalSection);



        // Signal the first event in the event array to tell the worker thread to
        // service an additional event in the event array
        if (WSASetEvent(EventArray[0]) == FALSE)
        {
            printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        else
            printf("Don't worry, WSASetEvent() is OK!\n");

    }
}

