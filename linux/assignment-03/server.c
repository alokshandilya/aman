#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CLIENTS 100
#define MAX_FRUITS 100
#define BUFFER_SIZE 1024

struct Fruit {
  char name[50];
  int quantity;
  char lastSold[50];
};

struct Client {
  struct sockaddr_in address;
  int hasTransaction;
};

struct Fruit fruitDatabase[MAX_FRUITS];
int numFruits = 0;
pthread_mutex_t databaseLock;
struct Client clients[MAX_CLIENTS];
int numClients = 0;
pthread_mutex_t clientsLock;

void addFruit(const char *name, int quantity, const char *lastSold) {
  pthread_mutex_lock(&databaseLock);

  if (numFruits < MAX_FRUITS) {
    strcpy(fruitDatabase[numFruits].name, name);
    fruitDatabase[numFruits].quantity = quantity;
    strcpy(fruitDatabase[numFruits].lastSold, lastSold);
    numFruits++;
  }

  pthread_mutex_unlock(&databaseLock);
}

void initializeFruits() {
  addFruit("apple", 50, "N/A");
  addFruit("banana", 75, "N/A");
  addFruit("orange", 75, "N/A");
}

void addClient(struct sockaddr_in clientAddr) {
  pthread_mutex_lock(&clientsLock);

  if (numClients < MAX_CLIENTS) {
    clients[numClients].address = clientAddr;
    clients[numClients].hasTransaction = 1; // Mark as having a transaction
    numClients++;
  }

  pthread_mutex_unlock(&clientsLock);
}

int sellFruit(const char *name, int quantitySold) {
  pthread_mutex_lock(&databaseLock);
  int hasTransaction = 0;

  for (int i = 0; i < numFruits; i++) {
    if (strcmp(fruitDatabase[i].name, name) == 0) {
      if (fruitDatabase[i].quantity >= quantitySold) {
        fruitDatabase[i].quantity -= quantitySold;
        printf("%d %s(s) sold. Remaining %s(s): %d\n", quantitySold, name, name,
               fruitDatabase[i].quantity);
        hasTransaction = 1;
      } else {
        printf("Not enough %s(s) in stock\n", name);
      }
      break;
    }
  }

  pthread_mutex_unlock(&databaseLock);
  return hasTransaction;
}

void *clientHandler(void *arg) {
  int clientSocket = *((int *)arg);
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  getpeername(clientSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

  char buffer[BUFFER_SIZE];
  int bytesRead = read(clientSocket, buffer, BUFFER_SIZE - 1);

  if (bytesRead > 0) {
    buffer[bytesRead] = '\0';

    char *name = strtok(buffer, ",");
    int quantity = atoi(strtok(NULL, ","));
    char *lastSold = strtok(NULL, ",");

    addFruit(name, quantity, lastSold);
    int hasTransaction = sellFruit(name, quantity); // Sell the fruit

    if (hasTransaction) {
      addClient(clientAddr); // Add the client to the list
    }

    const char *response = "Record added and fruit sold successfully.\n";
    write(clientSocket, response, strlen(response));

    // Send the total number of unique customers to the client
    char numUniqueCustomers[10];
    snprintf(numUniqueCustomers, sizeof(numUniqueCustomers), "%d", numClients);
    write(clientSocket, numUniqueCustomers, strlen(numUniqueCustomers));
  }

  close(clientSocket);
  pthread_exit(NULL);
}

void displayClients() {
  pthread_mutex_lock(&clientsLock);

  printf("Customer IDs who have done transactions:\n");
  for (int i = 0; i < numClients; i++) {
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clients[i].address.sin_addr), clientIP,
              INET_ADDRSTRLEN);
    int clientPort = ntohs(clients[i].address.sin_port);
    printf("%s:%d\n", clientIP, clientPort);
  }

  pthread_mutex_unlock(&clientsLock);
}

void handleSignal(int signal) {
  if (signal == SIGINT) {
    printf("\nReceived SIGINT. Displaying customer IDs:\n");
    displayClients();
    exit(0);
  }
}

int clientHasTransaction(const char *name, int quantity) {
  pthread_mutex_lock(&databaseLock);

  for (int i = 0; i < numFruits; i++) {
    if (strcmp(fruitDatabase[i].name, name) == 0) {
      if (fruitDatabase[i].quantity >= quantity) {
        pthread_mutex_unlock(&databaseLock);
        return 1; // Client has completed a transaction
      } else {
        pthread_mutex_unlock(&databaseLock);
        return 0; // Client didn't complete a transaction
      }
    }
  }

  pthread_mutex_unlock(&databaseLock);
  return 0; // Client didn't complete a transaction
}

int main() {
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);

  pthread_mutex_init(&databaseLock, NULL);
  pthread_mutex_init(&clientsLock, NULL);

  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(12345);

  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    perror("Socket binding failed");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  if (listen(serverSocket, 5) == -1) {
    perror("Listen failed");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  // Initialize the initial quantities of fruits
  initializeFruits();

  printf("Server listening on port 12345...\n");

  // Set up signal handler for Ctrl+C (SIGINT)
  signal(SIGINT, handleSignal);

  while (1) {
    clientSocket =
        accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
      perror("Accept failed");
      continue;
    }

    pthread_t tid;
    if (pthread_create(&tid, NULL, clientHandler, &clientSocket) != 0) {
      perror("Thread creation failed");
      close(clientSocket);
    }
  }

  pthread_mutex_destroy(&databaseLock);
  pthread_mutex_destroy(&clientsLock);
  close(serverSocket);

  return 0;
}
