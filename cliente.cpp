#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc == 1 || argc > 3) {
        cout << "Por favor siga o formato do programa: ./cliente SEU_IP PORTA" << endl;
        return 0;
    }
    
    string ipaddress = argv[1];
    int port = stoi(argv[2]);

    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cout << "Can't create socket" << endl;
        return 1;
    }

    // Create a hint structure for the server we're connecting with
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipaddress.c_str(), &hint.sin_addr);

    // Connect to the server on the socket
    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1) {
        return 1;
    }

    // While loop
    char buf[4096];
    string userInput;
    
    do {
        // Enter lines of text
        cout << "Message: ";
        getline(cin, userInput);

        // Send to server
        int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendResult == -1) {
            cout << "Could not send to server!" << endl;
            continue;
        }

        // Wait for response
        memset(buf, 0, 4096);
        int bytesReceived = recv(sock, buf, 4096, 0);

        // Display response
        cout << "Server: " << string(buf, bytesReceived) << endl;

    } while(true);

    // Close the socket

    return 0;
}