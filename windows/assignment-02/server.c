#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFFER_SIZE 1024

int main() {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    fprintf(stderr, "WSAStartup failed.\n");
    return 1;
  }

  SOCKET serverSocket;
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == INVALID_SOCKET) {
    fprintf(stderr, "Socket creation failed.\n");
    WSACleanup();
    return 1;
  }

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(12345);

  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    fprintf(stderr, "Socket binding failed.\n");
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  if (listen(serverSocket, 5) == SOCKET_ERROR) {
    fprintf(stderr, "Listen failed.\n");
    closesocket(serverSocket);
    WSACleanup();
    return 1;
  }

  printf("Server listening on port 12345...\n");

  while (1) {
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    SOCKET clientSocket;
    clientSocket =
        accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
      fprintf(stderr, "Accept failed.\n");
      continue;
    }

    char buffer[BUFFER_SIZE];
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead > 0) {
      buffer[bytesRead] = '\0';
      printf("Received from client: %s\n", buffer);

      const char *response = "Message received by the server.\n";
      send(clientSocket, response, strlen(response), 0);
    }

    closesocket(clientSocket);
  }

  closesocket(serverSocket);
  WSACleanup();

  return 0;
}
