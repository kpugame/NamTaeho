
#include<mutex>
#include<WinSock2.h>
#include<stdlib.h>
#include<stdio.h>
#include<list>

#include<atlimage.h>
/******Define*********/
#define SERVERPORT 9190
#define WINSIZEX 600
#define WINSIZEY 800
#define USERMAX 4

#define MAX_USER 2
#define MAX_ITEM 100
#define MAX_ENEMY 100
#define MAX_BULLET 1000

POINT PlaneImgSize, EnemyImgSize, BulletImgSize, ItemSize;

/************Enum************/
enum OBJTYPE { PLAYER = 1000, ENEMY, MISSILE };
enum UNITACTION {
	PLAYER1_LEFT_MOVE = 2000, PLAYER1_RIGHT_MOVE, PLAYER1_UP_MOVE, PLAYER1_DOWN_MOVE,
	PLAYER2_LEFT_MOVE = 3000, PLAYER2_RIGHT_MOVE, PLAYER2_UP_MOVE, PLAYER2_DOWN_MOVE,
	PLAYER1_FIRE = 4000, PLAYER2_FIRE
};
enum ITEMTYPE { LIFEITEM = 5000 };


/****************struct**********************/ 
struct UnitInfo // 29 Byte
{
	USHORT Type;// 유저 인지 적인지, 2 Byte 
	USHORT Action; // 무슨 행위인지 저장, 2 Byte
	BYTE HP;// 체력, 1 Byte
	POINT Pos; // 위치, 16 Byte
	RECT rt; // 충돌 체크, 16 Byte 
	USHORT AttackDelay;
};
struct BulletInfo
{
	USHORT Type;
	POINT pos;
	USHORT speed;
	RECT rt;
};

struct ItemInfo // 12 Byte
{
	USHORT ItemType; // 아이템 타입, 2 Byte
	POINT pos; // 아이템 위치, 8 Byte
	//USHORT ItemNum; // 아이템 개수, 2 Byte
	RECT rt;
};

struct UserInfo // 86 Byte
{
	SOCKET sock; // 소켓 구조체, 16 Byte
	UnitInfo UInfo[2]; // 유닛 정보, 29 Byte
	USHORT LIFE[2];
	//ItemInfo Item[2][3]; // 소유한 아이템, 36 Byte
	bool Leader, Init; // 방장인지 여부, 1 Byte
	size_t score[2]; // 게임 점수, 4 Byte
};

UserInfo User[MAX_USER];
ItemInfo Item[MAX_ITEM];
UnitInfo Enemy[MAX_ENEMY];
BulletInfo Bullet[MAX_BULLET];
//unsigned char count = 0;

USHORT UserCount = 0;
USHORT EnemyCount = 0;
USHORT ItemCount = 0;
USHORT BulletCount = 0;

HANDLE FrameHandle = NULL;

USHORT ClientBehavior = -1;
DWORD prevFrameTime = 0;
DWORD m_FrameInterval = 30;
DWORD prevFrameTime_ENEMY = 0;
DWORD m_FrameInterval_Monster = 5000;
DWORD prevFrameTime_Update = 0;
DWORD m_FrameInterval_Update = 30;
DWORD prevFrameTime_Item = 0;
DWORD m_FrameInterval_Item = 8000;

CImage tempimg;
CRITICAL_SECTION cs;
/*********************function**********************/
DWORD WINAPI FrameProcess(LPVOID arg);
void GameInit(void);
UnitInfo* CreateUnit(FILE* fp);
void GameBehaviorRecv(SOCKET* psock);
void GameOperationalProcess(USHORT Action);
void GameDataSend(SOCKET* psock);
void GameResultSend(SOCKET* psock);
// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags);

bool init = false;
bool update = true;

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

//클라이언트와의 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;

	//클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

	size_t DataLen;
	UserInfo info;
	USHORT Who;

	while (1)
	{
		retval = recvn(client_sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		if (client_sock == User[0].sock)
			Who = 0;
		else if (client_sock == User[1].sock)
			Who = 1;

		if (ClientBehavior == PLAYER1_FIRE)
		{
			Bullet[BulletCount].Type = PLAYER;
			Bullet[BulletCount].speed = 5;
			Bullet[BulletCount].pos.x = User[Who].UInfo[0].Pos.x + (PlaneImgSize.x * 0.4f);
			Bullet[BulletCount].pos.y = User[Who].UInfo[0].Pos.y;

			Bullet[BulletCount].rt.left = Bullet[BulletCount].pos.x;
			Bullet[BulletCount].rt.top = Bullet[BulletCount].pos.y;
			Bullet[BulletCount].rt.right = Bullet[BulletCount].rt.left + BulletImgSize.x;
			Bullet[BulletCount].rt.bottom = Bullet[BulletCount].rt.top + BulletImgSize.y;
			BulletCount++;
		}

		if (ClientBehavior == PLAYER2_FIRE)
		{
			Bullet[BulletCount].Type = PLAYER;
			Bullet[BulletCount].speed = 5;
			Bullet[BulletCount].pos.x = User[Who].UInfo[1].Pos.x + (PlaneImgSize.x * 0.4f);
			Bullet[BulletCount].pos.y = User[Who].UInfo[1].Pos.y;

			Bullet[BulletCount].rt.left = Bullet[BulletCount].pos.x;
			Bullet[BulletCount].rt.top = Bullet[BulletCount].pos.y;
			Bullet[BulletCount].rt.right = Bullet[BulletCount].rt.left + BulletImgSize.x;
			Bullet[BulletCount].rt.bottom = Bullet[BulletCount].rt.top + BulletImgSize.y;
			BulletCount++;
		}

		if (ClientBehavior >= PLAYER1_LEFT_MOVE && ClientBehavior <= PLAYER1_DOWN_MOVE)
		{
			switch (ClientBehavior)
			{
			case PLAYER1_LEFT_MOVE:
				//printf("PLAYER1_LEFT_MOVE\n");
				User[Who].UInfo[0].Pos.x -= 3;
				User[Who].UInfo[0].rt.left -= 3;
				User[Who].UInfo[0].rt.right -= 3;
				break;
			case PLAYER1_RIGHT_MOVE:
				//printf("PLAYER1_RIGHT_MOVE\n");
				User[Who].UInfo[0].Pos.x += 3;
				User[Who].UInfo[0].rt.left += 3;
				User[Who].UInfo[0].rt.right += 3;
				break;
			case PLAYER1_UP_MOVE:
				//printf("PLAYER1_UP_MOVE\n");
				User[Who].UInfo[0].Pos.y -= 3;
				User[Who].UInfo[0].rt.top -= 3;
				User[Who].UInfo[0].rt.bottom -= 3;
				break;
			case PLAYER1_DOWN_MOVE:
				//printf("PLAYER1_DOWN_MOVE\n");
				User[Who].UInfo[0].Pos.y += 3;
				User[Who].UInfo[0].rt.top += 3;
				User[Who].UInfo[0].rt.bottom += 3;
				break;
			}
		}
		else if (ClientBehavior >= PLAYER2_LEFT_MOVE && ClientBehavior <= PLAYER2_DOWN_MOVE)
		{
			switch (ClientBehavior)
			{
			case PLAYER2_LEFT_MOVE:
				//printf("PLAYER2_LEFT_MOVE\n");
				User[Who].UInfo[1].Pos.x -= 3;
				User[Who].UInfo[1].rt.left -= 3;
				User[Who].UInfo[1].rt.right -= 3;
				break;
			case PLAYER2_RIGHT_MOVE:
				//printf("PLAYER2_RIGHT_MOVE\n");
				User[Who].UInfo[1].Pos.x += 3;
				User[Who].UInfo[1].rt.left += 3;
				User[Who].UInfo[1].rt.right += 3;
				break;
			case PLAYER2_UP_MOVE:
				//printf("PLAYER2_UP_MOVE\n");
				User[Who].UInfo[1].Pos.y -= 3;
				User[Who].UInfo[1].rt.top -= 3;
				User[Who].UInfo[1].rt.bottom -= 3;
				break;
			case PLAYER2_DOWN_MOVE:
				//printf("PLAYER2_DOWN_MOVE\n");
				User[Who].UInfo[1].Pos.y += 3;
				User[Who].UInfo[1].rt.top += 3;
				User[Who].UInfo[1].rt.bottom += 3;
				break;
			}
		}
	}
	
	//closesocket()
	SOCKET delTemp = client_sock;
	closesocket(client_sock);
	for (int i = 0; i < UserCount; i++)
	{
		if (User[i].sock == delTemp)
		{
			ZeroMemory(&User[i], sizeof(UserInfo));
			UserCount--;
			break;
		}
	}
	printf("[TCP 서버] 클라이언트 종료 : IP 주소 = %s, 포트번호 = %d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	return 0;
}

//클라이언트와의 통신
DWORD WINAPI AllClientSendProcess(LPVOID arg)
{
	RECT tempRt;
	int retval;
	size_t DataLen;
	BOOL IsCollision = false;

	if (!init)
	{
		if(FrameHandle == NULL)
			FrameHandle = CreateThread(NULL, 0, FrameProcess, NULL, 0, NULL);
	}
	init = true;

	while (1)
	{	
		for (int i = 0; i < UserCount; i++)
		{
			if (!User[i].Init)
			{
				for (int j = 0; j < 2; j++)
				{
					User[i].score[j] = 0;
					User[i].Leader = true;
					/////////////////////////////
					User[i].UInfo[j].Type = ENEMY;
					User[i].UInfo[j].Pos.x = 200 * (j + 1);
					User[i].UInfo[j].Pos.y = 600;
					User[i].UInfo[j].Action = PLAYER1_UP_MOVE;
					User[i].UInfo[j].HP = 1;
					User[i].LIFE[j] = 1;

					User[i].UInfo[j].rt.left = User[i].UInfo[j].Pos.x;
					User[i].UInfo[j].rt.top = User[i].UInfo[j].Pos.y;
					User[i].UInfo[j].rt.right = User[i].UInfo[j].rt.left + PlaneImgSize.x;
					User[i].UInfo[j].rt.bottom = User[i].UInfo[j].rt.top + PlaneImgSize.y;
				}
				User[i].Init = true;
			}
		}
		if (GetTickCount() - prevFrameTime_Update > m_FrameInterval_Update)
		{
			if (((float)GetTickCount() - (float)prevFrameTime_Update) / 1000.f)
			{
				update = true;
			}
			prevFrameTime_Update = GetTickCount();
		}
		if (update)
		{

			for (int i = 0; i < UserCount; i++)
			{
				//총 접속 유저 숫자 전송
				retval = send(User[i].sock, (char*)&UserCount, sizeof(USHORT), 0);
				//printf("[소켓값 : %d]\n", User[i].sock);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}

				//데이터 길이 전송
				DataLen = sizeof(UserInfo) * UserCount;
				retval = send(User[i].sock, (char*)&DataLen, sizeof(size_t), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				//실제 데이터 전송
				//printf("%d %d %d\n", User[0].UInfo[0].Type, User[0].UInfo[1].Type, UserCount);
				retval = send(User[i].sock, (char*)User, DataLen, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				//printf("%d\n", User[0].sock);

				//총 모든비행기 숫자 전송
				retval = send(User[i].sock, (char*)&EnemyCount, sizeof(USHORT), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				//데이터 길이 전송
				DataLen = sizeof(UnitInfo) * EnemyCount;
				retval = send(User[i].sock, (char*)&DataLen, sizeof(size_t), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				//실제 데이터 전송
				retval = send(User[i].sock, (char*)&Enemy, DataLen, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}

				//총 아이템 숫자 전송
				retval = send(User[i].sock, (char*)&ItemCount, sizeof(USHORT), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				//데이터 길이 전송
				DataLen = sizeof(ItemInfo) * ItemCount;
				retval = send(User[i].sock, (char*)&DataLen, sizeof(size_t), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				//실제 데이터 전송
				retval = send(User[i].sock, (char*)&Item, DataLen, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}

				//총 미사일 숫자 전송
				retval = send(User[i].sock, (char*)&BulletCount, sizeof(USHORT), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				//미사일정보 길이 전송
				DataLen = sizeof(BulletInfo) * BulletCount;
				retval = send(User[i].sock, (char*)&DataLen, sizeof(size_t), 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				//실제 데이터 전송
				retval = send(User[i].sock, (char*)&Bullet, DataLen, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
			}
			
		}
		update = false;
	}
	
	return 0;
}


DWORD WINAPI FrameProcess(LPVOID arg)
{
	RECT tempRt;
	BOOL IsCollision = false;
	/*
	*/
	while (1)
	{
		if (GetTickCount() - prevFrameTime > m_FrameInterval)
		{
			if (((float)GetTickCount() - (float)prevFrameTime) / 1000.f)
			{
				for (int i = 0; i < EnemyCount; i++)
				{
					Enemy[i].AttackDelay--;
					//printf("Delay : %d\n", Enemy[i].AttackDelay);
					if (Enemy[i].AttackDelay <= 0)
					{
						Bullet[BulletCount].Type = ENEMY;
						Bullet[BulletCount].speed = 5;
						Bullet[BulletCount].pos.x = Enemy[i].Pos.x + (EnemyImgSize.x * 0.4f);
						Bullet[BulletCount].pos.y = Enemy[i].Pos.y + EnemyImgSize.y;

						Bullet[BulletCount].rt.left = Bullet[BulletCount].pos.x;
						Bullet[BulletCount].rt.top = Bullet[BulletCount].pos.y;
						Bullet[BulletCount].rt.right = Bullet[BulletCount].rt.left + BulletImgSize.x;
						Bullet[BulletCount].rt.bottom = Bullet[BulletCount].rt.top + BulletImgSize.y;
						BulletCount++;
						//printf("Enemy Fire\n");
						Enemy[i].AttackDelay = rand() % 5 + 10;
					}
				}

				for (int i = 0; i < EnemyCount; i++)
				{
					Enemy[i].Pos.y++;
					Enemy[i].rt.top++;
					Enemy[i].rt.bottom++;
				}
				for (int i = 0; i < ItemCount; i++)
				{
					Item[i].pos.y++;
					Item[i].rt.top++;
					Item[i].rt.bottom++;
				}
				for (int i = 0; i < ItemCount; i++)
				{
					for (int j = 0; j < UserCount; j++)
					{
						for (int k = 0; k < 2; k++)
						{
							IsCollision = IntersectRect(&tempRt, &Item[i].rt, &User[j].UInfo[k].rt);
							//printf("Item : [%d, %d] User : [%d, %d]\n", Item[i].rt.left, Item[i].rt.top, User[0].UInfo[0].rt.left, User[0].UInfo[0].rt.top);
							if (IsCollision)
							{
								printf("아이템충돌\nItem : [%d %d %d %d]\nUser : [%d %d %d %d]\n\n", 
									Item[i].rt.left, Item[i].rt.top, Item[i].rt.right, Item[i].rt.bottom, 
									User[j].UInfo[k].rt.left, User[j].UInfo[k].rt.top, User[j].UInfo[k].rt.right, User[j].UInfo[k].rt.bottom);
								for (int n = i; n < ItemCount - 1; n++)
								{
									Item[n] = Item[n + 1];
									//printf("LIFE : %d\n", User[j].LIFE[k]);
								}
								ItemCount--;
								User[j].LIFE[k]++;
							}
						}
					}
				}

				for (int i = 0; i < BulletCount; i++)
				{
					if (Bullet[i].Type == PLAYER)
					{
						Bullet[i].pos.y -= Bullet[i].speed;
						Bullet[i].rt.top -= Bullet[i].speed;
						Bullet[i].rt.bottom -= Bullet[i].speed;

						for (int j = 0; j < EnemyCount; j++)
						{
							IsCollision = IntersectRect(&tempRt, &Bullet[i].rt, &Enemy[j].rt);
							if (IsCollision)
							{
								//충돌
								//printf("충돌\n");
								for (int k = j; k < EnemyCount - 1; k++)
								{
									Enemy[k] = Enemy[k + 1];
								}
								EnemyCount--;
								//printf("EnemyCount : %d\n", EnemyCount);

								for (int n = i; n < BulletCount - 1; n++)
								{
									Bullet[n] = Bullet[n + 1];
								}
								BulletCount--;

								break;
							}
						}
					}
					else if (Bullet[i].Type == ENEMY)
					{
						Bullet[i].pos.y += Bullet[i].speed;
						Bullet[i].rt.top += Bullet[i].speed;
						Bullet[i].rt.bottom += Bullet[i].speed;

						for (int j = 0; j < UserCount; j++)
						{
							for (int k = 0; k < 2; k++)
							{
								IsCollision = IntersectRect(&tempRt, &Bullet[i].rt, &User[j].UInfo[k].rt);
								if (IsCollision)
								{
									//충돌
									//printf("충돌\n");

									//for (int k = j; k < EnemyCount - 1; k++)
									//{
									//	Enemy[k] = Enemy[k + 1];
									//}
									//EnemyCount--;
									//
									//printf("");

									for (int n = i; n < BulletCount - 1; n++)
									{
										Bullet[n] = Bullet[n + 1];
									}
									BulletCount--;

									break;
								}
							}
						}


					}

				}
			}

			prevFrameTime = GetTickCount();
		}
		if (GetTickCount() - prevFrameTime_ENEMY > m_FrameInterval_Monster)
		{
			if (((float)GetTickCount() - (float)prevFrameTime_ENEMY) / 1000.f)
			{
				Enemy[EnemyCount].Type = ENEMY;
				Enemy[EnemyCount].HP = 1;
				Enemy[EnemyCount].Action = 10003;
				Enemy[EnemyCount].Pos.x = rand() % 600;
				Enemy[EnemyCount].Pos.y = 0;
				Enemy[EnemyCount].AttackDelay = rand() % 5 + 10;

				Enemy[EnemyCount].rt.left = Enemy[EnemyCount].Pos.x;
				Enemy[EnemyCount].rt.top = Enemy[EnemyCount].Pos.y;
				Enemy[EnemyCount].rt.right = Enemy[EnemyCount].rt.left + EnemyImgSize.x;
				Enemy[EnemyCount].rt.bottom = Enemy[EnemyCount].rt.top + EnemyImgSize.y;

				EnemyCount++;
			}

			prevFrameTime_ENEMY = GetTickCount();
		}

		////////////
		if (GetTickCount() - prevFrameTime_Item > m_FrameInterval_Item)
		{
			if (((float)GetTickCount() - (float)prevFrameTime_Item) / 1000.f)
			{
				/*
				USHORT ItemType; // 아이템 타입, 2 Byte
				POINT pos; // 아이템 위치, 8 Byte
				USHORT ItemNum; // 아이템 개수, 2 Byte
				*/
				Item[ItemCount].ItemType = LIFEITEM;
				Item[ItemCount].pos.x = rand() % 600;
				Item[ItemCount].pos.y = 0;
			

				Item[ItemCount].rt.left = Item[ItemCount].pos.x;
				Item[ItemCount].rt.top = Item[EnemyCount].pos.y;
				Item[ItemCount].rt.right = Item[ItemCount].rt.left + ItemSize.x;
				Item[ItemCount].rt.bottom = Item[ItemCount].rt.top + ItemSize.y;
				ItemCount++;
				//Item[ItemCount].ItemNum
			}

			prevFrameTime_Item = GetTickCount();
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	User[0].Init = false;
	User[1].Init = false;
	int retval;
	tempimg.Load("Bullet.bmp");
	BulletImgSize.x = tempimg.GetWidth();
	BulletImgSize.y = tempimg.GetHeight();
	tempimg.Destroy();
	tempimg.Load("Player1.bmp");
	PlaneImgSize.x = tempimg.GetWidth();
	PlaneImgSize.y = tempimg.GetHeight();
	tempimg.Destroy();
	tempimg.Load("Enemy1.bmp");
	EnemyImgSize.x = tempimg.GetWidth();
	EnemyImgSize.y = tempimg.GetHeight();
	tempimg.Destroy();
	tempimg.Load("LIFEITEM.bmp");
	ItemSize.x = tempimg.GetWidth();
	ItemSize.y = tempimg.GetHeight();
	tempimg.Destroy();
	
	//PlaneImgSize.x = 
	InitializeCriticalSection(&cs);
	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	//socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	//bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	//listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	//데이터 통신에 사용 할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread, SendThread = NULL;
	bool check = false;
	
	////////////////////////////
	User[0].sock = NULL;
	User[1].sock = NULL;
	//UserCount = 1;
	//printf("소켓값 : %d\n", listen_sock);
	/////////////////////////////
	while (1)
	{
		//accept()
		addrlen = sizeof(clientaddr);
		//User[count] = new UserInfo;
		User[UserCount].sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (User[UserCount].sock == INVALID_SOCKET)
		{
			err_display("accept()");
			break;
		}
		else
			check = true;

		//접속한 클라이언트 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속 : IP 주소 = %s, 포트번호 = %d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		if (check)
		{
			if (SendThread == NULL)
			{
				SendThread = CreateThread(NULL, 0, AllClientSendProcess, NULL, 0, NULL);
			}
			//if (SendThread != NULL)
			//	CloseHandle(SendThread);
		}



		//스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)User[UserCount].sock, 0, NULL);
		if (hThread == NULL) {
			closesocket(client_sock);
		}
		else { CloseHandle(hThread);}
		UserCount++;
	}
	//closesocket()
	closesocket(listen_sock);

	//윈속 종료
	DeleteCriticalSection(&cs);
	WSACleanup();
	return 0;
}

void GameInit(void)
{
	/*
	각종서버정보를 초기화한다.
	*/
}

UnitInfo* CreateUnit(FILE* fp)
{
	/*
	유닛정보 얻어옴
		유닛생성
		유닛을 리턴해줌.리턴된 값은 리스트로 들어갈 예정.
	*/
	UnitInfo* obj = new UnitInfo;
	return obj;
}

void GameBehaviorRecv(SOCKET* psock)
{
	/*
	recvn()
		클라이언트의 행동에 대한 데이터를 받는다.
	*/
}

void GameOperationalProcess(USHORT Action)
{
	/*
	switch (Action)
	{
	case 왼쪽이동:
	case 오른쪽이동:
	case 위로이동:
	case 아래로 이동 :
	case 총알발사:
		case 아이템사용
			각 케이스에 맞는 연산 처리를 함
	}
	*/
}

void GameDataSend(SOCKET* psock)
{
	/*
	send()
		GameOperationalProcess()에서 처리한 결과를 각 클라에게 보낸다.
	*/
}
void GameResultSend(SOCKET* psock)
{
	/*
	Send()
		게임결과를 보내준다.
	*/
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
