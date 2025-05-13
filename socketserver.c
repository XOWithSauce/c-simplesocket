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

int __cdecl main(void) 
{
    WSADATA wsaData;
    int iResult;

    // tcp
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    // udp
    SOCKET SendSocket = INVALID_SOCKET;

    // structs for both
    struct addrinfo *resultTcp = NULL, 
                    hintsTcp;
    struct addrinfo *resultUdp = NULL, 
                    hintsUdp;

    // for UDP sendto function needs sockaddr, we fill this in TCP Accept
    SOCKADDR_IN clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    char message[DEFAULT_BUFLEN];

    
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
    hintsTcp.ai_flags = AI_PASSIVE;

    // for UDP
    ZeroMemory(&hintsUdp, sizeof(hintsUdp));
    hintsUdp.ai_family = AF_INET;
    hintsUdp.ai_socktype = SOCK_DGRAM;
    hintsUdp.ai_protocol = IPPROTO_UDP;
    hintsUdp.ai_flags = AI_PASSIVE;

    // Server address and port for TCP
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hintsTcp, &resultTcp);
    if (iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // For UDP we do the same
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hintsUdp, &resultUdp);
    if (iResult != 0 ) {
        printf("getaddrinfo (UDP) failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // TCP Socket
    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(resultTcp->ai_family, resultTcp->ai_socktype, resultTcp->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultTcp);
        WSACleanup();
        return 1;
    }

    // UDP Socket
    SendSocket = socket(resultUdp->ai_family, resultUdp->ai_socktype, resultUdp->ai_protocol);
    if (SendSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultTcp);
        freeaddrinfo(resultUdp);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, resultTcp->ai_addr, (int)resultTcp->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultTcp);
        freeaddrinfo(resultUdp);
        closesocket(ListenSocket);
        closesocket(SendSocket);
        WSACleanup();
        return 1;
    }

    // Same for UDP bind it
    iResult = bind(SendSocket, resultUdp->ai_addr, (int)resultUdp->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultTcp);
        freeaddrinfo(resultUdp);
        closesocket(ListenSocket);
        closesocket(SendSocket);
        WSACleanup();
        return 1;
    }
    
    
    freeaddrinfo(resultTcp);
    freeaddrinfo(resultUdp);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        closesocket(SendSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket, assign adress
    ClientSocket = accept(ListenSocket, (SOCKADDR *)&clientAddr, &clientAddrLen);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        closesocket(SendSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {
        // First receive client socket buffer on the TCP socket
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);
            // Echo the buffer back to the sender (tcp example from the example code)
            // iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            
            // Parse message and respond on the UDP socket
            char resp[20];
            int option = recvbuf[0] - '0';
            switch (option)
            {
            case 1:
                strcpy(resp, "Option1"); // Copy string into resp
                break;

            case 2:
                strcpy(resp, "Option2"); // Copy string into resp
                break;

            default:
                strcpy(resp, "Default"); // Copy string into resp
                break;
            }
            // Send response
            iSendResult = sendto(SendSocket, resp, strlen(resp), 0, (SOCKADDR *)&clientAddr, sizeof(clientAddr));
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                closesocket(SendSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }

        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            closesocket(SendSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // Close socket
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    closesocket(SendSocket);
    WSACleanup();
    return 0;
}