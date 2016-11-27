#include"TCPProcess.h"
#include"GameHeader.h"



void LobbyEnterRequestSend(SOCKET* psock, USHORT Action)
{
	//send()
	//�κ񿡰� ���� ��û
}

void LobbyEnterResultRecv(SOCKET* psock)
{
	//recvn()
	//�κ񿡼� ���ӿ�û�� ����� ����.
	//���� �� MessageBox() ȣ��
}
void LobbyUserDataRecv(SOCKET* psock)
{
	//recvn()
	//�κ� �ִ� �ٸ� �����鿡 ���� ������ ����.
	//���� ������ �̿��Ͽ� �κ� �ִ� �������������� ���
}
void LobbyuserBehaviorSend(SOCKET* psock, USHORT Action)
{
	//Send()
	//�κ� �ִ� ������ �ൿ������ ����.
}

void LobbyUserBehaviorResultRecv(SOCKET* psock)
{
	//recvn()
	//�κ񿡼� �� �ൿ�� ���� ����� ����.
	//���� ������ �����ϴٴ� �޽����� �޾����� ���ӽ���.
	//�ƴ϶�� ���ӽ��� ���� �޽����� ���.
}

void GameBehaviorSend(SOCKET* psock, USHORT Action)
{
	//send()
	//���� ���� �� ������ �ൿ�� ����.
}

void GameResultRecv(SOCKET* psock)
{
	//recvn()
	//���� ����� ���� ����
	//���� ����� ȭ�鿡 ���
}

void GameStartRecv(SOCKET* psock)
{
	//recvn()
	//���� ���� �޽����� ����. ���� �ʱ�ȭ ������ ����(��,�����ֵ�)

}

void GameDataRecv(SOCKET* psock)
{
	//recvn()
	//�������� �� ������ ����� ����.
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


// ����� ���� ������ ���� �Լ�
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