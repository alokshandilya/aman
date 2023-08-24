#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int server_socket, client_socket;
  struct sockaddr_in server_address, client_address;
  socklen_t client_length = sizeof(client_address);

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(12345); // Port number
  server_address.sin_addr.s_addr = INADDR_ANY;

  int opt = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
      -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  if (bind(server_socket, (struct sockaddr *)&server_address,
           sizeof(server_address)) == -1) {
    perror("Socket binding failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, 5) == -1) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server is listening...\n");

  do {
    client_socket = accept(server_socket, (struct sockaddr *)&client_address,
                           &client_length);
    if (client_socket == -1) {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }

    printf("Connected to client\n");

    // Receive expression from client
    float num1, num2, result;
    char operator;
    recv(client_socket, &num1, sizeof(num1), 0);
    recv(client_socket, &operator, sizeof(operator), 0);
    recv(client_socket, &num2, sizeof(num2), 0);

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

    send(client_socket, &result, sizeof(result), 0);
    close(client_socket);

    // Ask if the user wants to continue
    char choice;
    printf("Do you wish to continue (y/n)? ");
    scanf(" %c", &choice);

    if (choice != 'y' && choice != 'Y') {
      break; // Exit the loop if choice is not 'y' or 'Y'
    }
  } while (1);

  close(server_socket);

  return 0;
}
