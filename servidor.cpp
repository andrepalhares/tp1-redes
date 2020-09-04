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
    int posAleatoria = rand() % 78 + 1;
    int posLinha = 0;
    string palavraDaLinha;
    ifstream listaPalavras("palavras.txt");
    Palavra palavraSorteada;

    if(listaPalavras.is_open()) {
        while(getline(listaPalavras, palavraDaLinha)) {
            posLinha++;
            if (posLinha == posAleatoria) {
                palavraSorteada.palavra = palavraDaLinha;
                for (size_t i=0; i < palavraSorteada.palavra.length() - 1; i ++) {
                    palavraSorteada.descobertas.push_back(false);
                }
                cout << palavraSorteada.palavra << endl;
            }
        }
        listaPalavras.close();
    } else {
        cout << "ERRO: Falha ao abrir o arquivo de palavras. " << endl;
    }

    return palavraSorteada;
}

char converteParaBinario(string trecho) {
    char convertido = 0;
    
    for (int i = 0; i < 8; i++) {
        if (trecho[i] == '1') {
            convertido |= 1 << (7 - i);
        }
    }

    return convertido;
}

vector<int> retornaOcorrencias(string letra, string palavra) {
    vector<int> ocorrencias;
    char caractere = converteParaBinario(letra);

    for (size_t i = 0; i < palavra.size(); i++) {
        if (caractere == palavra[i]) {
            ocorrencias.push_back(i);
        }
    }

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

char* converteChar(int numero) {
    string stringTemporaria = bitset<8>(numero).to_string();

    char* numeroChar = new char[stringTemporaria.length()];

    strcpy(numeroChar, stringTemporaria.c_str());
    
    return numeroChar;
}

Mensagem criaMensagem(int numero) {
    Mensagem msg;
    msg.mensagem = converteChar(numero);

    return msg;
}

char* converteMensagemTipo3(vector<Mensagem> msg3) {
    char* msg;
    strcpy(msg, msg3[1].mensagem);
    
    for (size_t i = 2; i < msg3.size(); i++) {
        cout << msg3[i].mensagem << endl;

        // Adicionando espaçamento a mensagem para facilitar a compreensão do cliente
        strcat(msg, " ");
        strcat(msg, msg3[i].mensagem);
    }
    
    return msg;
}

int main(int argc, char* argv[]) {
    // Lendo os parâmetros pela linha de comando
    if (argc == 1 || argc > 2) {
        cout << "ERRO: Por favor siga o formato do programa: ./servidor PORTA" << endl;
        return 0;
    }
    
    //Inicializando parâmetros
    int port = stoi(argv[1]);

    // Criando o socket
    int sockerServidor = socket(AF_INET, SOCK_STREAM, 0);
    if (sockerServidor == -1) {
        cout << "ERRO: Não foi possível criar o socket" << endl;
        return 0;
    }

    // Associando o socket a IP e porta
    sockaddr_in enderecoServidor;
    enderecoServidor.sin_family = AF_INET;
    enderecoServidor.sin_port = htons(port);
    inet_pton(AF_INET, "0.0.0.0", &enderecoServidor.sin_addr);

    int bindResultado = bind(sockerServidor, (sockaddr*)&enderecoServidor, sizeof(enderecoServidor));
    if (bindResultado == -1) {
        cout << "ERRO: Não foi possível associar endereço ao socket" << endl;
        return 0;
    }

    // Aguardar chamadas
    int listenResultado = listen(sockerServidor, SOMAXCONN);
    if (listenResultado == -1) {
        cout << "ERRO: Não foi possível aguardar chamadas pelo método listen()" << endl;
        return 0;
    }

    // Aceitar chamada do cliente
    sockaddr_in cliente;
    socklen_t tamanhoCliente;
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clienteSocket = accept(sockerServidor, (sockaddr*)&cliente, &tamanhoCliente);
    if (clienteSocket == -1) {
        cout << "ERRO: Não foi possível conectar" << endl;
        return 0;
    }

    // Fechando o socket antigo do servidor
    close(sockerServidor);
    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    // Inicializando o jogo da forca
    Palavra palavraSorteada = sortearPalavra();
    bool gameOver;
    vector<Mensagem> msgTipo1;

    msgTipo1.push_back(criaMensagem(1));
    msgTipo1.push_back(criaMensagem(palavraSorteada.palavra.length() - 1));

    send(clienteSocket, msgTipo1[1].mensagem, 8, 0);

    // Iniciando a comunicação com o cliente
    char buffer[1024];
    vector<int> ocorrencias;
    vector<Mensagem> msgTipo3;
    vector<Mensagem> msgTipo4;

    while(true) {
        // Limpando buffer de possíveis lixos de memória
        memset(buffer, 0, 1024);

        // Espera até receber mensagem do Cliente
        int bytesRecebidos = recv(clienteSocket, buffer, 1024, 0);
        if (bytesRecebidos == -1) {
            cout << "ERRO: Não foi possível obter a mensagem" << endl;
            break;
        }
        if (bytesRecebidos == 0) {
            cout << "O Cliente desconectou" << endl;
            break;
        }

        // Determina as ocorrências
        string letraEscolhida = string(buffer, 0, bytesRecebidos);
        ocorrencias = retornaOcorrencias(letraEscolhida, palavraSorteada.palavra);

        // Criando mensagem tipo 3
        msgTipo3.push_back(criaMensagem(3));
        msgTipo3.push_back(criaMensagem(ocorrencias.size())); // ocorrencias é um array com as posicoes da letra
        // Cria a mensagem com as posicoes
        for (size_t i = 0; i < ocorrencias.size(); i++) {
            palavraSorteada.descobertas[ocorrencias[i]] = true;
            msgTipo3.push_back(criaMensagem(ocorrencias[i] + 1));
        }
        
        gameOver = checarSeAcabouJogo(palavraSorteada);
        if (gameOver) {
            // Se jogo tiver acabado, enviamos a mensagem 4
            msgTipo4.push_back(criaMensagem(4));

            send(clienteSocket, msgTipo4[0].mensagem, 8, 0);
            msgTipo4.clear();
        } else {
            // Se jogo ainda não tiver acabado, enviamos a mensagem 3
            char* mensagemEnviada = converteMensagemTipo3(msgTipo3);

            send(clienteSocket, mensagemEnviada, strlen(mensagemEnviada) + 1, 0);
            msgTipo3.clear();
        }
    }

    close(clienteSocket);

    return 0;
}
