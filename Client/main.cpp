#include <iostream> 
#include <thread> 
#include <WinSock2.h> 
#include <ws2tcpip.h> 
#include <stdio.h> 
#include <string> 

#pragma comment(lib, "Ws2_32.lib") 
using namespace std;

SOCKET clientSocket;

void receive() {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cerr << "Connection closed. Quitting" << endl;
            break;
        }
        cout << buffer << endl;
    }
}

int main() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0) {
        cerr << "Can't initialize winsock! Quitting" << endl;
        return -1;
    }

    string ipAddress;
    int port;
    cout << "Enter IP address: ";
    cin >> ipAddress;
    cout << "Enter port: ";
    cin >> port;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Can't create socket! Quitting" << endl;
        return -1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    if (connect(clientSocket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        cerr << "Can't connect to server! Quitting" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    // Ввод имени пользователя 
    string nickname;
    cin.ignore();
    cout << "Enter your nickname: ";
    getline(cin, nickname);

    // Отправка имени на сервер 
    send(clientSocket, nickname.c_str(), nickname.size() + 1, 0);

    // Создание потока для приема сообщений 
    thread receiveThread(receive);
    receiveThread.detach();

    // Отправка сообщений 
    char message[4096];
    cout << "You can start chating(enter 'q' to quit.)\n";
    while (true) {
        cin.getline(message, sizeof(message));

        if (message[0] == 'q' && message[1] == '\0') {
            send(clientSocket, message, strlen(message) + 1, 0);
            closesocket(clientSocket);
            break; // Выход при вводе 'q' 

        }

        send(clientSocket, message, strlen(message) + 1, 0);
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
