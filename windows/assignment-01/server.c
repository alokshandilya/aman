#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

int main() {
  WSADATA wsa;
  SOCKET server_socket, client_socket;
  struct sockaddr_in server_address, client_address;
  int client_length = sizeof(client_address);

  // Initialize Winsock
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    printf("Failed to initialize Winsock\n");
    return 1;
  }

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == INVALID_SOCKET) {
    printf("Socket creation failed\n");
    return 1;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(12345); // Port number
  server_address.sin_addr.s_addr = INADDR_ANY;

  // Reuse addr
  int opt = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt,
                 sizeof(opt)) == SOCKET_ERROR) {
    printf("setsockopt failed\n");
    return 1;
  }

  // Bind socket
  if (bind(server_socket, (struct sockaddr *)&server_address,
           sizeof(server_address)) == SOCKET_ERROR) {
    printf("Socket binding failed\n");
    return 1;
  }

  // Listen for incoming connections
  if (listen(server_socket, 5) == SOCKET_ERROR) {
    printf("Listen failed\n");
    return 1;
  }

  printf("Server is listening...\n");

  do {
    // Accept client connection
    client_socket = accept(server_socket, (struct sockaddr *)&client_address,
                           &client_length);
    if (client_socket == INVALID_SOCKET) {
      printf("Accept failed\n");
      return 1;
    }

    printf("Connected to client\n");

    // Receive expression from client
    float num1, num2, result;
    char operator;
    recv(client_socket, (char *)&num1, sizeof(num1), 0);
    recv(client_socket, &operator, sizeof(operator), 0);
    recv(client_socket, (char *)&num2, sizeof(num2), 0);

    // Perform calculation
    switch (operator) {
    case '+':
      result = num1 + num2;
      break;
    case '-':
      result = num1 - num2;
      break;
    case '*':
      result = num1 * num2;
      break;
    case '/':
      if (num2 != 0) {
        result = num1 / num2;
      } else {
        printf("Division by zero\n");
        result = 0.0;
      }
      break;
    default:
      printf("Invalid operator\n");
      result = 0.0;
      break;
    }

    // Send computed answer back to client
    send(client_socket, (char *)&result, sizeof(result), 0);

    // Close client socket
    closesocket(client_socket);

    // Ask if the user wants to continue
    char choice;
    printf("Do you wish to continue (y/n)? ");
    scanf(" %c", &choice);

    if (choice != 'y' && choice != 'Y') {
      break; // Exit the loop if choice is not 'y' or 'Y'
    }
  } while (1);

  // Close server socket
  closesocket(server_socket);
  WSACleanup();

  return 0;
}
