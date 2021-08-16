#pragma once

#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <random>
#include <fstream>

#pragma comment (lib , "ws2_32.lib")

#define BUFLEN 512
#define PORT 8888

void initWinsock(); // to initialise  Winsock and create a socket
void bindSocket();
void udpSend(char[], int);