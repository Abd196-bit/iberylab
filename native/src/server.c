#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void serve_file(int client_socket, const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (!file) {
        const char* not_found = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        send(client_socket, not_found, strlen(not_found), 0);
        return;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read file content
    char* buffer = (char*)malloc(file_size + 1);
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    // Send HTTP response
    char header[256];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", file_size);
    send(client_socket, header, strlen(header), 0);
    send(client_socket, buffer, file_size, 0);
    free(buffer);
}

void handle_request(int client_socket, const char* request, const char* base_dir) {
    char path[BUFFER_SIZE];
    sscanf(request, "GET %s HTTP/1.1", path);

    // Default to index.html if path is "/"
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }

    // Construct full file path
    char full_path[BUFFER_SIZE];
    sprintf(full_path, "%s%s", base_dir, path);

    // Serve the file
    serve_file(client_socket, full_path);
}

void start_server(const char* directory) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server running at http://localhost:%d\n", PORT);

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        char buffer[BUFFER_SIZE];
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        handle_request(client_socket, buffer, directory);
        close(client_socket);
    }

    close(server_socket);
}

int main() {
    start_server(".");
    return 0;
} 