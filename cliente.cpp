#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <bitset>
#include <vector>

using namespace std;

struct Mensagem {
    char* mensagem;
};

char* ConverteChar(int numero) {
    string temp_str = bitset<8>(numero).to_string();
    char* char_type = new char[temp_str.length()];
    strcpy(char_type, temp_str.c_str());
    
    return char_type;
}

Mensagem CriaMensagemTipo(int numero) {
    Mensagem msg;
    msg.mensagem = ConverteChar(numero);

    return msg;
}

Mensagem CriaMensagem(string letra) {
    char* cstr = new char [letra.length()+1];
    strcpy (cstr, letra.c_str());

    Mensagem msg;
    msg.mensagem = cstr;

    return msg;
}

int main(int argc, char* argv[]) {
    if (argc == 1 || argc > 3) {
        cout << "Por favor siga o formato do programa: ./cliente SEU_IP PORTA" << endl;
        return 0;
    }

    // cout << "existe? " << argv[1][0] << endl;
    
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

    // Recebendo primeira mensagem
    memset(buf, 0, 4096);
    int bytesReceived = recv(sock, buf, 4096, 0);
    cout << string(buf, bytesReceived) << endl;
    
    do {
        // Enter lines of text
        cout << "Message: "; // TODO: Apagar
        getline(cin, userInput);

        // Send to server
        // Tratar a mensagem, colocando um tipo
        vector<Mensagem> msgTipo2;

        msgTipo2.push_back(CriaMensagemTipo(2));
        msgTipo2.push_back(CriaMensagem(userInput));

        int sendResult = send(sock, msgTipo2[1].mensagem, 8, 0);
        if (sendResult == -1) {
            cout << "Could not send to server!" << endl;
            continue;
        }
        // if (bytesReceived == 0) {
        //     cout << "The server disconnected" << endl;
        //     break;
        // } NOT WORKING

        // Wait for response
        memset(buf, 0, 4096);
        int bytesReceived = recv(sock, buf, 4096, 0);
        string mensagemRecebida = string(buf, bytesReceived);
        string mensagemSucesso =  bitset<8>(4).to_string();

        // Display response
        cout << mensagemRecebida << endl;

        // TODO: Finalizando conexão quando acertar a palavra 
        if (mensagemSucesso == mensagemRecebida) {
            break;
        }

    } while(true);

    // Close the socket

    return 0;
}