#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_PRODUCTS 100
#define BUFFER_SIZE 1024

typedef struct {
    char name[50];
    int stock;
} Product;

Product warehouse[MAX_PRODUCTS];
int product_count = 0;

// Function to find product by name
int find_product(const char* name) {
    for (int i = 0; i < product_count; i++) {
        if (strcmp(warehouse[i].name, name) == 0)
            return i;
    }
    return -1;
}

// Process client commands
void process_command(int client_socket, char *command) {
    char response[BUFFER_SIZE];

    if (strncmp(command, "list", 4) == 0) {
        snprintf(response, sizeof(response), "Products in warehouse:\n");
        for (int i = 0; i < product_count; i++) {
            char product_info[100];
            snprintf(product_info, sizeof(product_info), "%s: %d\n", warehouse[i].name, warehouse[i].stock);
            strcat(response, product_info);
        }
        if (product_count == 0)
            strcat(response, "No products available.\n");

    } else if (strncmp(command, "create", 6) == 0) {
        char product_name[50];
        int initial_stock = 0;

        if (sscanf(command, "create %s %d", product_name, &initial_stock) < 1) {
            snprintf(response, sizeof(response), "Error: Invalid command format.\n");
        } else if (find_product(product_name) != -1) {
            snprintf(response, sizeof(response), "Error: Product already exists.\n");
        } else {
            strcpy(warehouse[product_count].name, product_name);
            warehouse[product_count].stock = initial_stock;
            product_count++;
            snprintf(response, sizeof(response), "Product created.\n");
        }

    } else if (strncmp(command, "add", 3) == 0) {
        char product_name[50];
        int amount;

        if (sscanf(command, "add %s %d", product_name, &amount) < 2) {
            snprintf(response, sizeof(response), "Error: Invalid command format.\n");
        } else {
            int index = find_product(product_name);
            if (index == -1) {
                snprintf(response, sizeof(response), "Error: Product not found.\n");
            } else {
                warehouse[index].stock += amount;
                snprintf(response, sizeof(response), "Product updated.\n");
            }
        }

    } else if (strncmp(command, "reduce", 6) == 0) {
        char product_name[50];
        int amount;

        if (sscanf(command, "reduce %s %d", product_name, &amount) < 2) {
            snprintf(response, sizeof(response), "Error: Invalid command format.\n");
        } else {
            int index = find_product(product_name);
            if (index == -1 || warehouse[index].stock < amount) {
                snprintf(response, sizeof(response), "Error: Insufficient stock or product not found.\n");
            } else {
                warehouse[index].stock -= amount;
                snprintf(response, sizeof(response), "Product updated.\n");
            }
        }

    } else if (strncmp(command, "remove", 6) == 0) {
        char product_name[50];
        if (sscanf(command, "remove %s", product_name) < 1) {
            snprintf(response, sizeof(response), "Error: Invalid command format.\n");
        } else {
            int index = find_product(product_name);
            if (index == -1 || warehouse[index].stock > 0) {
                snprintf(response, sizeof(response), "Error: Product not found or stock not zero.\n");
            } else {
                for (int i = index; i < product_count - 1; i++) {
                    warehouse[i] = warehouse[i + 1];
                }
                product_count--;
                snprintf(response, sizeof(response), "Product removed.\n");
            }
        }

    } else if (strncmp(command, "quit", 4) == 0) {
        snprintf(response, sizeof(response), "Goodbye!");
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;

    } else {
        snprintf(response, sizeof(response), "Error: Invalid command.\n");
    }
    send(client_socket, response, strlen(response), 0);
}

// Main function to start server
int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        int addrlen = sizeof(address);
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected.\n");

        int valread;
        while ((valread = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
            buffer[valread] = '\0';
            process_command(client_socket, buffer);
            memset(buffer, 0, BUFFER_SIZE);
        }
        printf("Client disconnected.\n");
        close(client_socket);
    }

    close(server_fd);
    return 0;
}
