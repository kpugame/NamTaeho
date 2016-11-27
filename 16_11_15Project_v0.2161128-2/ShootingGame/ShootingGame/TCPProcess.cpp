#include"TCPProcess.h"
#include"GameHeader.h"



void LobbyEnterRequestSend(SOCKET* psock, USHORT Action)
{
	//send()
	//로비에게 접속 요청
}

void LobbyEnterResultRecv(SOCKET* psock)
{
	//recvn()
	//로비에서 접속요청한 결과를 받음.
	//실패 시 MessageBox() 호출
}
void LobbyUserDataRecv(SOCKET* psock)
{
	//recvn()
	//로비에 있는 다른 유저들에 대한 정보를 받음.
	//받은 정보를 이용하여 로비에 있는 유저들의정보를 띄움
}
void LobbyuserBehaviorSend(SOCKET* psock, USHORT Action)
{
	//Send()
	//로비에 있는 유저의 행동정보를 보냄.
}

void LobbyUserBehaviorResultRecv(SOCKET* psock)
{
	//recvn()
	//로비에서 한 행동에 대한 결과를 받음.
	//게임 시작이 가능하다는 메시지를 받았으면 게임시작.
	//아니라면 게임시작 실패 메시지를 띄움.
}

void GameBehaviorSend(SOCKET* psock, USHORT Action)
{
	//send()
	//게임 진행 중 유저의 행동을 보냄.
}

void GameResultRecv(SOCKET* psock)
{
	//recvn()
	//게임 결과에 대해 받음
	//받은 결과를 화면에 띄움
}

void GameStartRecv(SOCKET* psock)
{
	//recvn()
	//게임 시작 메시지를 받음. 각종 초기화 정보를 포함(적,팀유닛등)

}

void GameDataRecv(SOCKET* psock)
{
	//recvn()
	//서버에서 한 연산의 결과를 받음.
}

//Error 
void err_quit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}