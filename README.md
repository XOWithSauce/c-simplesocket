# Basic TCP/UDP Socket Communication Example

Winsock implementation of TCP and UDP socket communication.

## Features
- **Logging:** The client logs received messages to a file (`log.txt`).

## Prerequisites
- Windows operating system.
- GCC compiler (MinGW recommended) or any other compiler that supports Winsock2.

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
