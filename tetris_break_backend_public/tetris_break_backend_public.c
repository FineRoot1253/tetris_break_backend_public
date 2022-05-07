#define _WINSOCKAPI_ 
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>
#include <time.h>
#include <stdint.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iptypes.h>
#include <iphlpapi.h>


#pragma comment(lib, "iphlpapi")
#pragma comment(lib, "ws2_32") 





#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))

#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#define BUF_SIZE 1024
#define MAX_CLNT 4
#define GAMESET 4
int light[4] = { 0,0,0,0 };
char name[20];
char roomName[20];
clock_t t1;
int* checkarr;
int ALLSCORE = 0;
char ASCORE[3];
unsigned WINAPI HandleClient(void* arg);//쓰레드 함수
unsigned WINAPI check(void* arg);
unsigned WINAPI check2(void* arg);
void SendMsg(char* msg, int len);//메시지 보내는 함수
void ErrorHandling(char* msg);//에러메시지 함수
void Wait(int sec);
int print_ipaddress();//서버내부IP + 네트워크어뎁터이름 호출함수
void print_addr(PIP_ADAPTER_UNICAST_ADDRESS ua); //서버내부IP 호출함수
void print_adapter(PIP_ADAPTER_ADDRESSES aa);//서버 네트워크 어뎁터이름 호출함수
int endcount;
char Tmsg[BUF_SIZE];
char msg[BUF_SIZE];
char seq[4][20] = { 0,0,0,0 };
char seq_cnt[4][2];
int clientCount = 0;
int clearCount = 0;
void arraysort(char* msg);//배열 정렬
void rearraysort();//배열 재정렬
void CheckCNT(char* msg, int j);//cnt보냄
int FULL = 0;
int GAMINGUSR = 0;
int R2USR = 0;
int GAMING = 0;
int IsR1 = 0;
int IsR2 = 0;

SOCKET clientSocks[MAX_CLNT];//클라이언트 소켓 보관용 배열
HANDLE iChat;
HANDLE udpSsend;
unsigned WINAPI Udpsend(void* arg);

WSADATA wsaData, udpWsaData;
SOCKET serverSock, clientSock, udpsenSock;
struct sockaddr_in sockaddr1;
struct sockaddr_in sockaddr2;
SOCKADDR_IN serverAddr, clientAddr, udpsenAddr;
int clientAddrSize;
HANDLE hThread;
char port[100];
char hostname[50];
char ipaddr[50];

int main() {
	memset(seq, NULL, sizeof(seq));

	LINGER ling = { 0, };
	ling.l_onoff = 1;
	ling.l_linger = 0;


	printf("Input server number : ");
	gets(port);
	printf("Input your Game Room Name : ");
	gets(roomName);
	udpSsend = (HANDLE)_beginthreadex(NULL, 0, Udpsend, 0, 0, NULL);
	/*
	if(argc!=2){
	printf("Usage : %s <port>\n",argv[0]);
	exit(1);
	}
	*/
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //윈도우 소켓을 사용하겠다는 사실을 운영체제에 전달
		ErrorHandling("WSAStartup() error!");

	//hMutex = CreateMutex(NULL, FALSE, NULL);//하나의 뮤텍스를 생성한다.
	serverSock = socket(PF_INET, SOCK_STREAM, 0); //하나의 소켓을 생성한다.
	setsockopt(serverSock, SOL_SOCKET, SO_LINGER, (CHAR*)&ling, sizeof(ling));
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(atoi(port));


	if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //생성한 소켓을 배치한다.
		ErrorHandling("bind() error");
	if (listen(serverSock, 5) == SOCKET_ERROR)//소켓을 준비상태에 둔다.
		ErrorHandling("listen() error");

	printf("listening...\n");

	while (1) {

		clientAddrSize = sizeof(clientAddr);
		clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//서버에게 전달된 클라이언트 소켓을 clientSock에 전달
		if (FULL == 1 || GAMING == 1)
			clientSock = -1;
		clientSocks[clientCount++] = clientSock;//클라이언트 소켓배열에 방금 가져온 소켓 주소를 전달
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&clientSock, 0, NULL);//HandleClient 쓰레드 실행, clientSock을 매개변수로 전달
		printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));

	}

	closesocket(serverSock);//생성한 소켓을 끈다.
	WSACleanup();//윈도우 소켓을 종료하겠다는 사실을 운영체제에 전달
	return 0;
}

unsigned WINAPI HandleClient(void* arg) {
	char* result;
	SOCKET clientSock = *((SOCKET*)arg); //매개변수로받은 클라이언트 소켓을 전달
	int strLen = 0, i, j = 0;

	while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) { //클라이언트로부터 메시지를 받을때까지 기다린다.

		if (strstr(msg, "GAMEOVER")) {
			Sleep(100);
			sprintf(Tmsg, "GAMEOVER");
			IsR1 = 0;
			IsR2 = 0;


		}
		if (strstr(msg, "ENDGAME")) {
			endcount++;
			if (endcount == clientCount)
				exit(1);
		}

		if (strstr(msg, "NAME"))
			arraysort(msg);
		if (strstr(msg, "GAMESTART")) {
			GAMINGUSR++;
			if (GAMINGUSR == clientCount) {
				GAMING = 1;
				IsR1 = 1;
				iChat = (HANDLE)_beginthreadex(NULL, 0, check, 0, 0, NULL);
			}

		}
		if (strstr(msg, "R2START")) {
			R2USR++;
			if (R2USR == clientCount) {
				sprintf(Tmsg, "R2START");
				IsR1 = 0;
				IsR2 = 1;
				iChat = (HANDLE)_beginthreadex(NULL, 0, check2, 0, 0, NULL);
			}

		}
		if (strstr(msg, "SYSTEMCNT")) {
			CheckCNT(msg, j);
		}

		if (strstr(msg, "R1CLEAR")) {
			clearCount++;
			printf("claer count = %d, client count = %d", clearCount, clientCount);
			if (clearCount == clientCount) {
				sprintf(Tmsg, "NEXTROUND");
				//_endthreadex(0);
				IsR1 = 0;
			}
			else {
				result = strtok(msg, ":");
				sprintf(Tmsg, "%d:CRCNT", clearCount);


			}
		}


		if (!strcmp(msg, "q")) {
			send(clientSock, "q", 1, 0);
			break;
		}
		if (strLen == SOCKET_ERROR) {
			printf("recv() error \n");
			break;
		}
		SendMsg(Tmsg, *Tmsg);//SendMsg에 받은 메시지를 전달한다.
		sprintf(Tmsg, "\0");
	}

	printf("client left the chat\n");

	//이 줄을 실행한다는 것은 해당 클라이언트가 나갔다는 사실임 따라서 해당 클라이언트를 배열에서 제거해줘야함
	for (i = 0; i < clientCount; i++) {//배열의 갯수만큼
		if (clientSock == clientSocks[i]) {//만약 현재 clientSock값이 배열의 값과 같다면
			j = i;
			while (j < clientCount) {//클라이언트 개수 만큼
				clientSocks[j] = clientSocks[j + 1];//앞으로 땡긴다.
				if (j == 3)
					sprintf(seq[j], "\0");
				else {
					sprintf(seq[j], "%s", seq[j + 1]);
					sprintf(seq[j + 1], "\0");
				}
				j++;
			}
			break;

		}
	}
	rearraysort();
	clientCount--;//클라이언트 개수 하나 감소

	closesocket(clientSock);//소켓을 종료한다.

	return 0;

}

void SendMsg(char* msg, int len) { //메시지를 모든 클라이언트에게 보낸다.
	int i;
	printf("%.*s \n", len, msg);
	for (i = 0; i < clientCount; i++)//클라이언트 개수만큼
		send(clientSocks[i], msg, len, 0);//클라이언트들에게 메시지를 전달한다.
}
void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
void arraysort(char* msg) {//배열정리와 순서 전달
	char* result;
	int j = 0;
	while (j < 4) {
		if (seq[j][0] == NULL) {
			result = strtok(msg, ":");
			sprintf(seq[j], "%s", result);
			if (j == 0)
				sprintf(Tmsg, "%s: SEQUNCE1\n", seq[0]);
			else if (j == 1)
				sprintf(Tmsg, "%s:%s SEQUNCE2\n", seq[0], seq[1]);
			else if (j == 2)
				sprintf(Tmsg, "%s:%s:%s: SEQUNCE3\n", seq[0], seq[1], seq[2]);
			else if (j == 3) {
				sprintf(Tmsg, "%s:%s:%s:%s: SEQUNCE4\n", seq[0], seq[1], seq[2], seq[3]);
				FULL = 1;
			}
			break;
		}
		else
			j++;


	}


}
void rearraysort() {//배열 재정렬
	int j = clientCount - 1;
	while (j-- >= 0) {
		if (j == 2) {
			sprintf(Tmsg, "%s:%s:%s: RESORT3\n", seq[0], seq[1], seq[2]);
			FULL = 0;
			break;
		}
		else if (j == 1) {
			sprintf(Tmsg, "%s:%s RESORT2\n", seq[0], seq[1]);
			break;
		}
		else if (j == 0) {
			sprintf(Tmsg, "%s: RESORT1\n", seq[0]);
			break;
		}
	}
	SendMsg(Tmsg, *Tmsg);

}
void CheckCNT(char* msg, int j) { // 점수 받아서 전달
	char* result;
	char mmsg[100];
	if (strstr(msg, seq[0])) {
		result = strtok(msg, ":");
		sprintf(seq_cnt[0], "%s", result);
		sprintf(Tmsg, "%s:%s: SYSTEMCNT\n", seq_cnt[0], seq[0]);
	}
	else if (strstr(msg, seq[1])) {
		result = strtok(msg, ":");
		sprintf(seq_cnt[1], "%s", result);
		sprintf(Tmsg, "%s:%s: SYSTEMCNT\n", seq_cnt[1], seq[1]);
	}
	else if (strstr(msg, seq[2])) {
		result = strtok(msg, ":");
		sprintf(seq_cnt[2], "%s", result);
		sprintf(Tmsg, "%s:%s: SYSTEMCNT\n", seq_cnt[2], seq[2]);
	}
	else if (strstr(msg, seq[3])) {
		result = strtok(msg, ":");
		sprintf(seq_cnt[3], "%s", result);
		sprintf(Tmsg, "%s:%s: SYSTEMCNT\n", seq_cnt[3], seq[3]);
	}

	if (IsR2) {
		sprintf(mmsg, Tmsg);
		SendMsg(mmsg, *mmsg);
		ALLSCORE++;
		sprintf(Tmsg, "%d:ALLSCORE", ALLSCORE);

	}
}
unsigned WINAPI check2() {
	int i = 0;
	while (IsR2) {
		if (i % 2 == 0) {
			sprintf(msg, "THUNDERON");
			SendMsg(msg, *msg);
			Wait(6);
			i++;
		}
		if (i % 2 == 1) {
			sprintf(msg, "THUNDEROFF");
			SendMsg(msg, *msg);
			Wait(6);
			i++;
		}
	}
}
unsigned WINAPI check() {

	while (IsR1) {
		int i = 0;//조건 변수
		if (i % clientCount == 0) {
			sprintf(msg, "%s: LIGHTON", seq[i % clientCount]);
			SendMsg(msg, *msg);
			Wait(6);
			sprintf(msg, "%s: LIGHTOFF", seq[i % clientCount]);
			SendMsg(msg, *msg);
			i++;
		}
		if (i % clientCount == 1) {
			sprintf(msg, "%s: LIGHTON", seq[i % clientCount]);
			SendMsg(msg, *msg);
			Wait(6);
			sprintf(msg, "%s: LIGHTOFF", seq[i % clientCount]);
			SendMsg(msg, *msg);
			i++;
		}
		if (i % clientCount == 2) {
			sprintf(msg, "%s: LIGHTON", seq[i % clientCount]);
			SendMsg(msg, *msg);
			Wait(6);
			sprintf(msg, "%s: LIGHTOFF", seq[i % clientCount]);
			SendMsg(msg, *msg);
			i++;
		}
		if (i % clientCount == 3) {
			sprintf(msg, "%s: LIGHTON", seq[i % clientCount]);
			SendMsg(msg, *msg);
			Wait(6);
			sprintf(msg, "%s: LIGHTOFF", seq[i % clientCount]);
			SendMsg(msg, *msg);
			i++;
		}
	}

}
void Wait(int sec) {
	clock_t end;
	end = clock() + sec * CLOCKS_PER_SEC;
	while (clock() < end) {}
}

//udpsend()부분 스레드함수화 해야함
unsigned WINAPI Udpsend(void* arg) {

	WSADATA wsaData;
	SOCKET hSendSock;
	int state, so_broadcast = TRUE;

	char name[256] = "localhost";
	char buf[BUF_SIZE];
	char* address;

	struct hostent* pHostEnt;
	struct in_addr in;
	struct sockaddr_in broadAddr;
	DWORD nMyIP, nMySubMask, nTemp1, nTemp2;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	printf("trying: socket...\n");
	if ((hSendSock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		ErrorHandling("socket() error!");

	gethostname(name, 256);
	pHostEnt = gethostbyname(name);
	in.s_addr = ((struct in_addr*)pHostEnt->h_addr)->s_addr;
	address = inet_ntoa(in);
	printf("trying: setsockopt...\n");

	nMyIP = inet_addr(address);
	nMySubMask = inet_addr("255.255.255.0");
	nTemp1 = nMyIP & nMySubMask;
	nTemp2 = nMySubMask ^ 0XFFFFFFFF;

	memset(&broadAddr, 0, sizeof(broadAddr));
	broadAddr.sin_family = AF_INET;
	broadAddr.sin_port = htons(atoi(port));
	broadAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	print_ipaddress();
	sprintf(buf, "%s:%s", ipaddr, roomName);
	state = setsockopt(hSendSock, SOL_SOCKET, SO_BROADCAST, (char*)&so_broadcast, sizeof(broadAddr));
	if (state == SOCKET_ERROR)
		ErrorHandling("setsocket() error!");


	printf("%.*s ", strlen(buf), buf);

	while (1) {
		sendto(hSendSock, buf, strlen(buf), 0, (SOCKADDR*)&broadAddr, sizeof(broadAddr));
		if (GAMING == 1)//게임변수가 1이되면 udpsend스레드가 꺼진다
			break;
	}
	closesocket(hSendSock);
	WSACleanup();
}

char adaptName[20];
void print_adapter(PIP_ADAPTER_ADDRESSES aa)
{
	char buf[BUFSIZ];
	memset(buf, 0, BUFSIZ);
	WideCharToMultiByte(CP_ACP, 0, aa->FriendlyName, wcslen(aa->FriendlyName), buf, BUFSIZ, NULL, NULL);
	if (strstr(buf, "이더넷"))
		strcpy(adaptName, buf);
	if (strstr(buf, "로컬"))
		strcpy(adaptName, buf);
	if (strstr(buf, "Wi-Fi"))
		strcpy(adaptName, buf);
	printf("adapter_name:%s\n", buf);

}

void print_addr(PIP_ADAPTER_UNICAST_ADDRESS ua)
{
	char buf[BUFSIZ];

	int family = ua->Address.lpSockaddr->sa_family;
	char str[10];

	int i;
	int cnt = 0;
	printf("\t%s ", family == AF_INET ? "IPv4" : "IPv6");
	strcpy(str, family == AF_INET ? "IPv4" : "IPv6");

	memset(buf, 0, BUFSIZ);
	getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
	printf("%s\n", buf);
	if (!strcmp(str, "IPv4")) {
		if (strstr(adaptName, "이더넷"))
			strcpy(ipaddr, buf);
		else if (strstr(adaptName, "로컬"))
			strcpy(ipaddr, buf);
		else if (strstr(adaptName, "Wi-Fi"))
			strcpy(ipaddr, buf);

		printf("%s\n", ipaddr);
		adaptName[0] = '\0';
	}
}

int print_ipaddress()
{
	DWORD rv, size;
	PIP_ADAPTER_ADDRESSES adapter_addresses, aa;
	PIP_ADAPTER_UNICAST_ADDRESS ua;

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size);
	if (rv != ERROR_BUFFER_OVERFLOW) {
		fprintf(stderr, "GetAdaptersAddresses() failed...");
		return 1;
	}
	adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(size);

	rv = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapter_addresses, &size);
	if (rv != ERROR_SUCCESS) {
		fprintf(stderr, "GetAdaptersAddresses() failed...");
		free(adapter_addresses);
		return 1;
	}

	for (aa = adapter_addresses; aa != NULL; aa = aa->Next) {
		print_adapter(aa);
		for (ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next) {
			print_addr(ua);
		}
	}

	free(adapter_addresses);
	return 0;
}
