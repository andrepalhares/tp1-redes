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

char* converteChar(int numero) {
    string stringTemporaria = bitset<8>(numero).to_string();

    char* numeroChar = new char[stringTemporaria.length()];

    strcpy(numeroChar, stringTemporaria.c_str());
    
    return numeroChar;
}

Mensagem criaMensagemTipo(int numero) {
    Mensagem msg;
    msg.mensagem = converteChar(numero);

    return msg;
}

Mensagem criaMensagem(string letra) {
    char* mensagemChar = new char [letra.length() + 1];
    strcpy (mensagemChar, letra.c_str());

    Mensagem msg;
    msg.mensagem = mensagemChar;

    return msg;
}

int main(int argc, char* argv[]) {
    if (argc == 1 || argc > 3) {
        // Lendo os parâmetros pela linha de comando
        cout << "ERRO: Por favor siga o formato do programa: ./cliente SEU_IP PORTA" << endl;
        return 0;
    }
    
    //Inicializando parâmetros
    string enderecoIP = argv[1];
    int porta = stoi(argv[2]);

    // Criando um socket
    int socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socketCliente == -1) {
        cout << "ERRO: Não foi possível criar o socket" << endl;
        return 0;
    }

    // Associando o socket a IP e porta
    sockaddr_in endereco;
    endereco.sin_family = AF_INET;
    endereco.sin_port = htons(porta);
    inet_pton(AF_INET, enderecoIP.c_str(), &endereco.sin_addr);

    // Tentando se conectar ao servidor
    int connectRes = connect(socketCliente, (sockaddr*)&endereco, sizeof(endereco));
    if (connectRes == -1) {
        return 0;
    }

    // Iniciando o jogo da forca
    vector<Mensagem> msgTipo2;
    char buffer[1024];
    string palpite;

    // Recebendo primeira mensagem
    memset(buffer, 0, 1024);
    int bytesRecebidos = recv(socketCliente, buffer, 1024, 0);
    if (bytesRecebidos == -1) {
        cout << "ERRO: Não foi possível obter a mensagem" << endl;
        return 0;
    }
    if (bytesRecebidos == 0) {
        cout << "O Servidor se desconectou" << endl;
        return 0;
    }

    string mensagem1 = string(buffer, bytesRecebidos);
    cout << mensagem1 << endl;
    
    do {
        // Cliente deve dar um palpite
        cout << "Message: "; // TODO: Apagar
        getline(cin, palpite);

        // Tratar a mensagem, colocando um tipo antes de enviar para o Servidor
        msgTipo2.push_back(criaMensagemTipo(2));
        msgTipo2.push_back(criaMensagem(palpite));

        int sendResult = send(socketCliente, msgTipo2[1].mensagem, 8, 0);
        msgTipo2.clear();

        if (sendResult == -1) {
            cout << "ERRO: Não foi possível enviar mensagem ao servidor" << endl;
            continue;
        }

        memset(buffer, 0, 1024);
        int bytesRecebidos = recv(socketCliente, buffer, 1024, 0);
        if (bytesRecebidos == -1) {
            cout << "ERRO: Não foi possível obter a mensagem" << endl;
            break;
        }
        if (bytesRecebidos == 0) {
            cout << "O Servidor desconectou" << endl;
            break;
        }

        // Exibindo mensagem recebida
        string mensagemRecebida = string(buffer, bytesRecebidos);
        cout << mensagemRecebida << endl;

        // Ao acertar a palavra, o cliente recebe uma mensagem do tipo 4 
        if (bitset<8>(4).to_string() == mensagemRecebida) {
            break;
        }

    } while(true);

    close(socketCliente);

    return 0;
}