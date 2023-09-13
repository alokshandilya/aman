#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
  int clientSocket;
  struct sockaddr_in serverAddr;

  // Create socket
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Setup server address
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(12345); // Use the same port as the server
  serverAddr.sin_addr.s_addr =
      inet_addr("127.0.0.1"); // Use the server's IP address

  // Connect to the server
  if (connect(clientSocket, (struct sockaddr *)&serverAddr,
              sizeof(serverAddr)) == -1) {
    perror("Connection failed");
    close(clientSocket);
    exit(EXIT_FAILURE);
  }

  // Gather fruit information from user
  char name[50];
  int quantity;
  char lastSold[50];

  printf("Enter fruit name: ");
  scanf("%s", name);
  printf("Enter quantity: ");
  scanf("%d", &quantity);
  printf("Enter last sold timestamp: ");
  scanf("%s", lastSold);

  // Create message to send to server
  char message[BUFFER_SIZE];
  snprintf(message, BUFFER_SIZE, "%s,%d,%s", name, quantity, lastSold);

  // Send message to server
  write(clientSocket, message, strlen(message));

  // Receive response from server
  char response[BUFFER_SIZE];
  int bytesRead = read(clientSocket, response, BUFFER_SIZE - 1);
  if (bytesRead > 0) {
    response[bytesRead] = '\0';
    printf("Server response: %s\n", response);
  }

  // Receive and display the total number of unique customers
  bytesRead = read(clientSocket, response, BUFFER_SIZE - 1);
  if (bytesRead > 0) {
    response[bytesRead] = '\0';
    printf("Total unique customers: %s\n", response);
  }

  close(clientSocket);

  return 0;
}
