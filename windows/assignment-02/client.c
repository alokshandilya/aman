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

  SOCKET clientSocket;
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == INVALID_SOCKET) {
    fprintf(stderr, "Socket creation failed.\n");
    WSACleanup();
    return 1;
  }

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(12345);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(clientSocket, (struct sockaddr *)&serverAddr,
              sizeof(serverAddr)) == SOCKET_ERROR) {
    fprintf(stderr, "Connection failed.\n");
    closesocket(clientSocket);
    WSACleanup();
    return 1;
  }

  char message[BUFFER_SIZE];
  printf("Enter a message to send to the server: ");
  fgets(message, BUFFER_SIZE, stdin);

  send(clientSocket, message, strlen(message), 0);

  char response[BUFFER_SIZE];
  int bytesRead = recv(clientSocket, response, BUFFER_SIZE - 1, 0);
  if (bytesRead > 0) {
    response[bytesRead] = '\0';
    printf("Server response: %s\n", response);
  }

  closesocket(clientSocket);
  WSACleanup();

  return 0;
}
