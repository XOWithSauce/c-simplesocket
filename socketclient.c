#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "11111"


void writeLog(char msg[])
{
    FILE *logfile;
    logfile = fopen("log.txt", "a");
    if (logfile)
    {
        fprintf(logfile, "%s\n", msg);
    }
    fclose(logfile);
}

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    int iResult;
    
    // tcp
    SOCKET ConnectSocket = INVALID_SOCKET;
    // udp
    SOCKET ReceiveSocket = INVALID_SOCKET;

    // structs for both
    struct addrinfo *resultTcp = NULL,
                    *ptrTcp = NULL,
                    hintsTcp;

    struct addrinfo *resultUdp = NULL,
                    hintsUdp;

    // No need to parse getaddr
    SOCKADDR_IN serverAddr;

    // Variables for recvfrom
    struct sockaddr_in senderAddr;
    int senderAddrLen = sizeof(senderAddr);

    const char *sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // for TCP
    ZeroMemory(&hintsTcp, sizeof(hintsTcp));
    hintsTcp.ai_family = AF_INET;
    hintsTcp.ai_socktype = SOCK_STREAM;
    hintsTcp.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port for TCP
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hintsTcp, &resultTcp);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptrTcp=resultTcp; ptrTcp != NULL ;ptrTcp=ptrTcp->ai_next) {
        ConnectSocket = socket(ptrTcp->ai_family, ptrTcp->ai_socktype,
            ptrTcp->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket (TCP) failed with error: %ld\n", WSAGetLastError());
            continue;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptrTcp->ai_addr, (int)ptrTcp->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        // Connection successful, copy server address for udp socket
        if (ptrTcp->ai_addrlen <= sizeof(serverAddr)) {
            memcpy(&serverAddr, ptrTcp->ai_addr, ptrTcp->ai_addrlen);
        } else {
            printf("Server address is too large to copy.\n");
            closesocket(ConnectSocket);
            freeaddrinfo(resultTcp);
            WSACleanup();
            return 1;
        }

        break; // Connected
    }

    freeaddrinfo(resultTcp);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Get the local address
    SOCKADDR_IN localTcpAddr;
    int localTcpAddrLen = sizeof(localTcpAddr);
    if (getsockname(ConnectSocket, (SOCKADDR *)&localTcpAddr, &localTcpAddrLen) == SOCKET_ERROR) {
        printf("getsockname (TCP) failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // UDP Socket
    ReceiveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ReceiveSocket == INVALID_SOCKET) {
        printf("socket (UDP) failed with error: %ld\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Bind the UDP socket to the same local address and port as the TCP connection
    iResult = bind(ReceiveSocket, (SOCKADDR *)&localTcpAddr, sizeof(localTcpAddr));
    if (iResult == SOCKET_ERROR) {
        printf("Client UDP bind failed with error: %d\n", WSAGetLastError());
        closesocket(ReceiveSocket);
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    int s;
    // Basic menu
    do {
        printf("Select an option\n");
        printf("1) Exit\n");
        printf("2) Send and Read\n");
        printf("Type a number: ");
        scanf("%d", &s);

        switch (s) {
            case 1:
                // shutdown the connection
                iResult = shutdown(ConnectSocket, SD_SEND);
                if (iResult == SOCKET_ERROR) {
                    printf("shutdown failed with error: %d\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    closesocket(ReceiveSocket);
                    WSACleanup();
                    return 1;
                }
                break;
            case 2:
                // Type a message and send it on tcp, read with udp
                char msg[DEFAULT_BUFLEN];
                printf("Message: ");
                getchar();
                scanf("%s", msg);
                // Send a buffer on the TCP socket
                iResult = send(ConnectSocket, msg, (int)strlen(msg), 0);
                if (iResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    closesocket(ReceiveSocket);
                    WSACleanup();
                    return 1;
                }

                printf("Bytes Sent: %ld\n", iResult);

                // Receive buffer on UDP
                senderAddrLen = sizeof(senderAddr);
                iResult = recvfrom(ReceiveSocket, recvbuf, recvbuflen, 0, (struct sockaddr*)&senderAddr, &senderAddrLen);
                if (iResult > 0) {
                    printf("Bytes received (UDP): %d\n", iResult);
                    recvbuf[iResult] = '\0';
                    printf("Message received: %s\n", recvbuf);
                    // Write log
                    writeLog(recvbuf);
                }
                break;
            default:
                printf("Incorrect option.\n");
                break;
        }
    } while (s != 1 || iResult > 0);

    // cleanup
    closesocket(ConnectSocket);
    closesocket(ReceiveSocket);
    WSACleanup();
    return 0;
}
