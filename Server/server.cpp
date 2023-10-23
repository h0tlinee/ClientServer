#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>  
#include <string>  
#include <thread>  
#include <vector>  
#include <WinSock2.h>  
#include <ws2tcpip.h>  
#include <stdio.h> 

#pragma comment(lib, "Ws2_32.lib") 
using namespace std;

SOCKET clients[100];
string nicknames[100];
string clientIPs[100];
int clientCount = 0;

void broadcast(const string& message, int senderIndex) {
    for (int i = 0; i < clientCount; ++i) {
        if (i != senderIndex && clients[i] != INVALID_SOCKET) {
            send(clients[i], message.c_str(), message.size() + 1, 0);
        }
    }
}

void clientHandler(SOCKET clientSocket, int clientIndex) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (buffer[0] == 'q' && buffer[1] == '\0') {
            cout << "User " << nicknames[clientIndex] << " disconnected." << endl;
            string leaveMessage = nicknames[clientIndex] + " left the chat.";
            broadcast(leaveMessage, clientIndex);
            clients[clientIndex] = INVALID_SOCKET;
            break;
        }
        if (bytesReceived < 0) {
            cerr << "Error in recv. Quitting" << endl;
            clients[clientIndex] = INVALID_SOCKET;
            break;
        }
        string message = nicknames[clientIndex] + ": " + string(buffer);
        broadcast(message, clientIndex);
        cout << message << endl;
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

    int port=2004;
  /*  cout << "Enter port: ";
    cin >> port;*/

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        cerr << "Can't create socket! Quitting" << std::endl;
        return -1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    char str[256];
    inet_ntop(AF_INET, &hint.sin_addr, str, 256);
    listen(listening, 2);

    char hostt[256];
    char HostName[1024]; //создаем буфер для имени хоста
    if (!gethostname(HostName, 1024)) //получаем имя хоста
    {
        if (LPHOSTENT lphost = gethostbyname(HostName)) //получаем IP сервера
            strcpy(hostt, inet_ntoa(*((in_addr*)lphost->h_addr_list[0])));
        //преобразуем переменную типа LPIN_ADDR в DWORD
    }

    cout << "Server started at port:"<<htons(hint.sin_port)<<" IP: "<<hostt<< ".Waiting for connections..." << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(listening, (sockaddr*)&clientAddr, &clientSize);

        char host[NI_MAXHOST];
        char service[NI_MAXSERV];

        memset(host, 0, NI_MAXHOST);
        memset(service, 0, NI_MAXSERV);

      

        if (getnameinfo((sockaddr*)&clientAddr, sizeof(clientAddr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
            cout << host << " connected on port "<< service << " IP:" << inet_ntoa(clientAddr.sin_addr)<<endl;
            clientIPs[clientCount] = host;
        }
        else {
            inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
            cout << host << " connected on port " << ntohs(clientAddr.sin_port) << " IP:" << inet_ntoa(clientAddr.sin_addr)<<std::endl;
            clientIPs[clientCount] = host;
        }

        recv(clientSocket, host, NI_MAXHOST, 0);
        string nickname = string(host);
        nicknames[clientCount] = nickname;

        cout << "User " << nickname << " connected." << endl;
        string joinMessage = nickname + " joined the chat."+" IP: " + inet_ntoa(clientAddr.sin_addr);
        broadcast(joinMessage, clientCount);
        clients[clientCount] = clientSocket;
        thread clientThread(clientHandler, clientSocket, clientCount);
        clientThread.detach();
        clientCount++;
    }
    closesocket(listening);
    WSACleanup();
    return 0;
}
