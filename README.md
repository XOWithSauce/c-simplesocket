# Basic TCP/UDP Socket Communication Example

Winsock implementation of TCP and UDP socket communication.

## Features
- **Logging:** The client logs received messages to a file (`log.txt`).
- Uses Port 11111 by default
- Default buffer length of 512

## Prerequisites
- Windows operating system.
- GCC compiler (MinGW recommended) or any other compiler that supports Winsock2.

## Configure
- Modify the `#define DEFAULT_BUFLEN` line in both the server and client code to allow for larger buffer size (Default 512)
- Modify the `#define DEFAULT_PORT` line in both the server and client code to allow for different port usage
  
## Build Instructions
To compile the client and server programs, use the following commands:

```bash
gcc socketclient.c -o socketclient -lws2_32
gcc socketserver.c -o socketserver -lws2_32
```

To run the server:

```bash
./socketserver
```

To run the client:

```bash
./socketclient <server-ip>
```

## Attribution
This implementation is based on the following Microsoft Winsock documentation:
- [Complete Client Code](https://learn.microsoft.com/en-us/windows/win32/winsock/complete-client-code)
- [Complete Server Code](https://learn.microsoft.com/en-us/windows/win32/winsock/complete-server-code)
