#pragma once
#include"GameHeader.h"

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags);
void err_quit(char* msg);
void err_display(char* msg);


void LobbyEnterRequestSend(SOCKET* psock, USHORT Action);
void LobbyEnterResultRecv(SOCKET* psock);
void LobbyUserDataRecv(SOCKET* psock);
void LobbyuserBehaviorSend(SOCKET* psock, USHORT Action);
void LobbyUserBehaviorResultRecv(SOCKET* psock);
void GameBehaviorSend(SOCKET* psock, USHORT Action);
void GameResultRecv(SOCKET* psock);
void GameStartRecv(SOCKET* psock);
void GameDataRecv(SOCKET* psock);
