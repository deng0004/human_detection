#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>

int main() {
    // Create a socket for the client
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Failed to create client socket" << std::endl;
        return 1;
    }

    // Connect to the server
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to connect to the server" << std::endl;
        close(clientSocket);
        return 1;
    }

    std::cout << "Connected to the server" << std::endl;

    // Main loop for receiving messages from the server
    while (true) {
        // Receive message from the server
        char buffer[1024];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Failed to receive message from the server" << std::endl;
            break;
        }

        // Display the received message
        std::string message(buffer, bytesRead);
        std::cout << "Received message from server: " << message << std::endl;
    }

    // Close the client socket
    close(clientSocket);

    return 0;
}
