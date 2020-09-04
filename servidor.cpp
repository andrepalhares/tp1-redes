#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <fstream>
#include <time.h>
#include <list>
#include <vector>
#include <sstream>
#include <bitset>

using namespace std;

struct Mensagem {
    char* mensagem;
};

struct Palavra {
    string palavra;
    vector<bool> descobertas;
};

Palavra sortearPalavra() {
    srand((unsigned)time(NULL));
    int rand_index = rand() % 78 + 1;
    int line_index = 0;
    string line;
    ifstream myFile("palavras.txt");
    Palavra palavraSorteada;

    if(myFile.is_open()) {
        while(getline(myFile, line)) {
            line_index++;
            if (line_index == rand_index) {
                palavraSorteada.palavra = line;
                for (size_t i=0; i < palavraSorteada.palavra.length() - 1; i ++) {
                    palavraSorteada.descobertas.push_back(false);
                }
                cout << palavraSorteada.palavra << endl;
                cout << palavraSorteada.palavra.length() - 1 << endl;
            }
        }
        myFile.close();
    } else {
        cout << "File failed " << endl;
    }

    return palavraSorteada;
}

char ConverteParaBinario(string str) {
    char parsed = 0;
    
    for (int i = 0; i < 8; i++) {
        if (str[i] == '1') {
            parsed |= 1 << (7 - i);
        }
    }

    return parsed;
}

Palavra letraExisteNaPalavra(string letra, Palavra palavraSorteada) {
    // int codigo = stoi(letra);
    // char caractere = char(codigo);
    char caractere = ConverteParaBinario(letra);

    int posicao = palavraSorteada.palavra.find(caractere);

    if(posicao >= 0 && posicao <= palavraSorteada.palavra.size()) {
        palavraSorteada.descobertas[posicao] = true;

        for(size_t i = posicao + 1; i < palavraSorteada.palavra.size(); i++) {
            if(caractere == palavraSorteada.palavra[i]) {
                palavraSorteada.descobertas[i] = true;
            }
        }
    }

    return palavraSorteada;
}

vector<int> retornaOcorrencias(string letra, string palavra) {
    vector<int> ocorrencias;
    char caractere = ConverteParaBinario(letra);

    for (size_t i = 0; i < palavra.size(); i++) {
        if (caractere == palavra[i]) {
            ocorrencias.push_back(i);
        }
    }

    cout << "ocorrencias: " << ocorrencias.size() << endl;
    return ocorrencias;
}

bool checarSeAcabouJogo(Palavra palavraSorteada) {
    for (size_t i = 0; i < palavraSorteada.descobertas.size(); i++) {
        if (palavraSorteada.descobertas[i] == false) {
            return false;
        }
    }

    return true;
}

char* ConverteChar(int numero) {
    string temp_str = bitset<8>(numero).to_string();
    char* char_type = new char[temp_str.length()];
    strcpy(char_type, temp_str.c_str());
    
    return char_type;
}

Mensagem CriaMensagem(int numero) {
    Mensagem msg;
    msg.mensagem = ConverteChar(numero);

    return msg;
}

char* ConverteMensagemTipo3(vector<Mensagem> msg3) {
    char* msg;
    strcpy(msg, msg3[1].mensagem);
    
    for (size_t i = 2; i < msg3.size(); i++) {
        cout << msg3[i].mensagem << endl;
        strcat(msg, " ");
        strcat(msg, msg3[i].mensagem);
    }
    
    return msg;
}

int main(int argc, char* argv[]) {
    // Lendo os parâmetros pela linha de comando
    if (argc == 1 || argc > 2) {
        cout << "Por favor siga o formato do programa: ./servidor PORTA" << endl;
        return 0;
    }
    
    //Inicializando parâmetros
    int port = stoi(argv[1]);
    bool gameOver;

    //Sorteando a palavra
    Palavra palavraSorteada = sortearPalavra();

    // Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1) {
        cerr << "Error 1: Can't create a socket";
        return -1;
    }

    // Bind the socket to a IP / port
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) {
        cerr << "Error 2: Can't bind to IP/Port";
        return -2;
    }

    // Mark the socket for listening in
    if (listen(listening, SOMAXCONN) == -1) {
        cerr << "Erorr 3: Can't listen";
        return -3;
    }

    // Accept a call
    sockaddr_in client;
    socklen_t clientSize;
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    if (clientSocket == -1) {
        cerr << "Error 4: Can't connect";
        return -4;
    }

    // Close the listening socket
    close(listening);
    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    if (result) {
        vector<Mensagem> msgTipo1;

        msgTipo1.push_back(CriaMensagem(1));
        msgTipo1.push_back(CriaMensagem(palavraSorteada.palavra.length() - 1));

        send(clientSocket, msgTipo1[1].mensagem, 8, 0);
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on " << ntohs(client.sin_port) << endl;
    }

    // While recieving display message, echo message
    char buf[4096];
    vector<int> ocorrencias;
    vector<Mensagem> msgTipo3;
    vector<Mensagem> msgTipo4;
    while(true) {
        // Clear the buffer
        memset(buf, 0, 4096);

        // Wait for a message
        int bytesRecv = recv(clientSocket, buf, 4096, 0);
        if (bytesRecv == -1) {
            cerr << "Error 5: Connection issue" << endl;
            break;
        }
        if (bytesRecv == 0) {
            cout << "The client disconnected" << endl;
            break;
        }

        // Display message
        // cout << "Received: " << string(buf, 0, bytesRecv) << endl;
        
        // TODO: Jogo de forca
        // palavraSorteada = letraExisteNaPalavra(string(buf, 0, bytesRecv), palavraSorteada);

        // Determina as ocorrências
        ocorrencias = retornaOcorrencias(string(buf, 0, bytesRecv), palavraSorteada.palavra);
        // Cria mensagem tipo 3
        msgTipo3.push_back(CriaMensagem(3));
        // Cria mensagem com a quantidade de ocorrências
        msgTipo3.push_back(CriaMensagem(ocorrencias.size()));
        // Cria a mensagem com as posicoes
        for (size_t i = 0; i < ocorrencias.size(); i++) {
            palavraSorteada.descobertas[ocorrencias[i]] = true;
            msgTipo3.push_back(CriaMensagem(ocorrencias[i] + 1));
        }
        
        if (checarSeAcabouJogo(palavraSorteada)) {
            msgTipo4.push_back(CriaMensagem(4));

            send(clientSocket, msgTipo4[0].mensagem, 8, 0);
            msgTipo4.clear();
        } else {
            char* mensagemEnviada = ConverteMensagemTipo3(msgTipo3);

            send(clientSocket, mensagemEnviada, strlen(mensagemEnviada) + 1, 0);
            msgTipo3.clear();
        }
    }

    // Close socket
    close(clientSocket);

    return 0;
}
