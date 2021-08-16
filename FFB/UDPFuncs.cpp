#include "UDPFuncs.h"

SOCKET socketObj;
struct sockaddr_in server, si_other, destination_addr;
int slen, recv_len;
char buf[BUFLEN];
WSADATA wsa;

void initWinsock() {
	slen = sizeof(si_other);
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");
	if ((socketObj = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		wprintf(L"could not create socket: %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("socket created \n");
	server.sin_family = AF_INET;
	server.sin_addr.S_un.S_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	destination_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &destination_addr.sin_addr.s_addr);
	destination_addr.sin_port = htons(PORT);
	
}
void bindSocket() {
	if (bind(socketObj, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("bind failed with error : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("bind done \n");
}
void udpSend(char buf[], int messageLen) {
	if (sendto(socketObj, buf, messageLen,0 ,(SOCKADDR*)&destination_addr, sizeof(destination_addr)) == SOCKET_ERROR) {
		wprintf(L"send failed with error: %ld", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("message sent succesfully\n");

}