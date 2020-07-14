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

using namespace std;

list<int> letraExisteNaPalavra(string letra, string palavra) {
    list<int> listaPosicoes;
    int codigo = stoi(letra);
    char caractere = char(codigo);

    int posicao = palavra.find(caractere);

    if(posicao > 0 && posicao <= palavra.size()) {
        listaPosicoes.push_back(posicao + 1);

        for(size_t i = posicao + 1; i < palavra.size(); i++) {
            if(caractere == palavra[i]) {
                listaPosicoes.push_back(i + 1);
            }
        }
    }

    return listaPosicoes;

}

void showlist(list <int> g) 
{ 
    list <int> :: iterator it; 
    for(it = g.begin(); it != g.end(); ++it) 
        cout << '\t' << *it; 
    cout << '\n'; 
} 


list<int> retornaPosicoes(string palavra, char letra) {
    list <int> posicoes;
    int i = 0;
    for(char& c : palavra) {
        i++;
        if (c == letra) {
            posicoes.push_back(i);
        }
    }
    return posicoes;
}

int main(int argc, char* argv[]) {
    // Lendo os parâmetros pela linha de comando
    if (argc == 1 || argc > 2) {
        cout << "Por favor siga o formato do programa: ./servidor PORTA" << endl;
        return 0;
    }
    
    int port = stoi(argv[1]);
    srand((unsigned)time(NULL));
    int rand_index = rand() % 78 + 1;
    string palavra;

    // Lendo arquivo de palavras
    int line_index = 0;
    string line;
    ifstream myFile("palavras.txt");

    if(myFile.is_open()) {
        while(getline(myFile, line)) {
            line_index++;
            if (line_index == rand_index) {
                palavra = line;
                cout << palavra << endl;
                cout << palavra.length() - 1 << endl;
            }
        }
        myFile.close();
    } else {
        cout << "File failed " << endl;
    }

    // Funcao pra retornar as posicoes da letra escolhida na palavra sorteada
    int a = 97 ;
    cout << (char)a << endl;
    list<int> posicoes = retornaPosicoes(palavra, char(a));
    showlist(posicoes); // posicoes.size() para retornar a quantidade de ocorrências

    // Salvar ocorrências para determinar quando o jogo acabou

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
        cout << host << " connected on " << svc << endl;
        char testePalavra[255] = "qualquer coisa";
        send(clientSocket, testePalavra, 255, 0);
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on " << ntohs(client.sin_port) << endl;
    }

    // While recieving display message, echo message
    char buf[4096];
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
        cout << "Received: " << string(buf, 0, bytesRecv) << endl;
        
        // TODO: Jogo de forca
        letraExisteNaPalavra(string(buf, 0, bytesRecv), palavra);

        // Resend message
        send(clientSocket, buf, bytesRecv + 1, 0);

    }

    // Close socket
    close(clientSocket);

    return 0;
}
