#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

int main() {
  WSADATA wsa;
  SOCKET client_socket;
  struct sockaddr_in server_address;

  // Initialize Winsock
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    printf("Failed to initialize Winsock\n");
    return 1;
  }

  // Create socket
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == INVALID_SOCKET) {
    printf("Socket creation failed\n");
    return 1;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(12345);                  // Server port
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address

  // Connect to server
  if (connect(client_socket, (struct sockaddr *)&server_address,
              sizeof(server_address)) == SOCKET_ERROR) {
    printf("Connection failed\n");
    return 1;
  }

  printf("Connected to server\n");

  // Send expression to server
  float num1, num2;
  char operator;

  printf("Enter first number: ");
  scanf("%f", &num1);
  printf("Enter operator (+, -, *, /): ");
  scanf(" %c", &operator);
  printf("Enter second number: ");
  scanf("%f", &num2);

  send(client_socket, (char *)&num1, sizeof(num1), 0);
  send(client_socket, &operator, sizeof(operator), 0);
  send(client_socket, (char *)&num2, sizeof(num2), 0);

  // Receive computed answer from server
  float result;
  recv(client_socket, (char *)&result, sizeof(result), 0);
  printf("Result: %.2f\n", result);

  // Close socket
  closesocket(client_socket);
  WSACleanup();

  return 0;
}
