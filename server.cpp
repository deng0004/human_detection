#include <iostream>
#include <opencv2/opencv.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/ioctl.h>




#define SAMPLE_RATE 44100
#define DURATION_SECONDS 0.5
#define AMPLITUDE 0.5

// Function to send a message to the client
void sendMessage(int clientSocket, const std::string& message) {
    send(clientSocket, message.c_str(), message.length(), 0);
}



// Function to play a TTS voice using eSpeak and ALSA
void playVoice(const std::string& message) {
    std::string command = "espeak -ven+f3 -s150 \"" + message + "\" --stdout | aplay -D default";
    system(command.c_str());
}



int main() {
    // Create a socket for the server
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create server socket" << std::endl;
        return 1;
    }

    // Bind the socket to a specific address and port
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind server socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) == -1) {
        std::cerr << "Failed to listen for connections" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server listening on port 8080" << std::endl;

    // Accept a client connection
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == -1) {
        std::cerr << "Failed to accept client connection" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Client connected" << std::endl;

    // Load the face cascade classifier
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load("data/haarcascade_frontalface_default.xml")) {
        std::cerr << "Failed to load face cascade classifier" << std::endl;
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    // Main loop for face detection
    cv::VideoCapture videoCapture(0); // Initialize video capture device

    bool isFaceDetected = false;

    while (true) {
        cv::Mat frame; // Capture a frame from the video source
        videoCapture >> frame;

        // Perform face detection
        std::vector<cv::Rect> faces;
        cv::Mat frameGray;
        cv::cvtColor(frame, frameGray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(frameGray, frameGray);
        faceCascade.detectMultiScale(frameGray, faces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));

        // Process the detected faces
        for (const auto& face : faces) {
            // Send a message to the client
            std::string message = "HUMAN DETECTED";
            sendMessage(clientSocket, message);

            // Draw a rectangle around the face
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);

            // Set the flag indicating a face is detected
            isFaceDetected = true;
        }

        // If no face is detected, play a normal melody; otherwise, play a surprise melody
        if (isFaceDetected) {
            // Play the TTS voice
            playVoice("HUMAN DETECTED");
            isFaceDetected = false;  // Reset the flag
        } else {
            // Send a message to the client
            std::string message = "NOT A HUMAN";
            // Play the TTS voice
            playVoice("NOT A HUMAN");
            sendMessage(clientSocket, message);
        }

        // Display the frame
        cv::imshow("HUMAN DETECTED", frame);

        // Check for user input to exit the loop
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Close the client and server sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}
