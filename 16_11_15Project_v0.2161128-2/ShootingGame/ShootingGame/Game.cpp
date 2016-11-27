#include"GameHeader.h"
#include"WindowPrepare.h"
#define SERVERIP "127.0.0.1"
#define SERVERPORT 9190

#pragma comment(linker , "/entry:WinMainCRTStartup /subsystem:console")

#define MAX_USER 2
#define MAX_ITEM 100
#define MAX_ENEMY 100
#define MAX_BULLET 1000
#define WINSIZEX 600
#define WINSIZEY 800

//Define
HANDLE hRecvEvent, hSendEvent;
//Global Variable
SOCKET sock;
HWND Connect;
HANDLE hTCPThread, hGameDataThread;
UserInfo info;

//Method
DWORD WINAPI TCPProcess(LPVOID arg);
DWORD WINAPI GameDataProcess(LPVOID arg);

Plane* CreatePlane();
void Init(void);
Plane* CreateObject(void);
static DWORD prevFrameTime = 0;
static DWORD m_FrameInterval = 30;

static bool gamestart = false;
static bool IsSend = false;
///////////////////////////////////////

UserInfo User[MAX_USER];
ItemInfo Item[MAX_ITEM];
UnitInfo Enemy[MAX_ENEMY];
BulletInfo Bullet[MAX_BULLET];

//USHORT 

USHORT UserCount = 0;
USHORT EnemyCount = 0;
USHORT ItemCount = 0;
USHORT BulletCount = 0;

USHORT ClientBehavior = -1;

void Input_Process();
void KeyEventOn();
static int retvalue;
#define MAX_FIRE_COUNT 5
int firecount1 =MAX_FIRE_COUNT, firecount2 = MAX_FIRE_COUNT;
bool P1Fire = false,P2Fire = false;
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	//변수선언
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	static RECT ClntRt;
	static HBITMAP bitmap;
	//static std::list<Plane> Unit;
	static POINT MapPos;
	static CImage map,Player[4];
	static CImage EnemyImg[MAX_ENEMY];
	static CImage BulletImg[MAX_BULLET];
	static CImage ItemImg[MAX_ITEM], Player_Life;
	char strbuf[20];
	static TCHAR str[256];

	switch (iMessage)
	{
	case WM_CREATE:
		hSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hSendEvent == NULL) return 1;
		hRecvEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hRecvEvent == NULL) return 1;

		//////////////////////////////////////
		hWndMain = hWnd;
		Connect = CreateWindow("button", "Connect", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			20, 20, 100, 25, hWnd, (HMENU)0, g_hInst, NULL);
		map.Load("map.bmp");

		for (int i = 0; i < 4; i++)
		{
			wsprintf(strbuf, "Player%d.bmp", i+1);
			Player[i].Load(strbuf);
		}
		MapPos.x = 0; MapPos.y = 0;
		SetTimer(hWnd, 1, 40, NULL);
		for (int i = 0; i < MAX_ENEMY; i++)
		{
			wsprintf(strbuf, "Enemy1.bmp");
			EnemyImg[i].Load(strbuf);
		}
		for (int i = 0; i < MAX_BULLET; i++)
		{
			//wsprintf(strbuf, "Player%d.bmp", i + 1);
			BulletImg[i].Load("Bullet.bmp");
		}
		for (int i = 0; i < MAX_ITEM; i++)
		{
			//wsprintf(strbuf, "Player%d.bmp", i + 1);
			ItemImg[i].Load("LIFEITEM.bmp");
		}
		Player_Life.Load("LIFEITEM.bmp");
		return 0;

	case WM_COMMAND:
		switch (wParam)
		{
			case 0:
				hTCPThread = CreateThread(NULL, 0, TCPProcess, NULL, 0, NULL);
				SetTimer(hWnd, 2, 16, NULL);

				DestroyWindow(Connect);
				break;
		}
		return 0;
	case WM_KEYDOWN:
		//SetEvent(hSendEvent);
		return 0;
	case WM_TIMER:
		switch (wParam)
		{
			case 1:
				MapPos.y++;
				if (MapPos.y > 800)
					MapPos.y = 0;
				KeyEventOn();
				if (firecount1 > 0)
					firecount1--;
				else
				{
					firecount1 = MAX_FIRE_COUNT;
					P1Fire = true;
				}
				if (firecount1 > 0)
					firecount2--;
				else
				{
					firecount2 = MAX_FIRE_COUNT;
					P2Fire = true;
				}
				InvalidateRect(hWnd, &ClntRt, NULL);
				break;
			
			case 2:
				SetEvent(hRecvEvent);
				break;
			
		}
		
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWndMain, &ClntRt); //클라이언트 작업영역을 가져오고
										
		hMemDC = CreateCompatibleDC(hdc);
		bitmap = CreateCompatibleBitmap(hdc, ClntRt.right, ClntRt.bottom);

		SelectObject(hMemDC, bitmap);
		FillRect(hMemDC, &ClntRt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		
		if (gamestart)
		{
			//맵스크롤
			map.BitBlt(hMemDC, 0, MapPos.y, map.GetWidth(), map.GetHeight() + MapPos.y, 0, 0, SRCCOPY);
			map.BitBlt(hMemDC, 0, 0, map.GetWidth(), map.GetHeight(), 0, WINSIZEY - MapPos.y, SRCCOPY);
			for (int i = 0, n = 0; i < UserCount; i++)
			{
				for (int j = 0; j < 2; j++, n++)
				{
					Player[n].TransparentBlt(hMemDC, User[i].UInfo[j].Pos.x, User[i].UInfo[j].Pos.y, Player[n].GetWidth(), Player[n].GetHeight(),
						0, 0,Player[n].GetWidth(), Player[n].GetHeight(), RGB(255, 0, 255));

					Player[n].TransparentBlt(hMemDC, 20+(j*450), 700 + (i*30), Player[n].GetWidth()/2, Player[n].GetHeight()/2,
						0, 0, Player[n].GetWidth(), Player[n].GetHeight(), RGB(255, 0, 255));
					//Rectangle(hMemDC, User[i].UInfo[j].rt.left, User[i].UInfo[j].rt.top, User[i].UInfo[j].rt.right, User[i].UInfo[j].rt.bottom);
					//printf("Player : %d %d", User[i].LIFE[0]);
					wsprintf(str, " X %d", User[i].LIFE[j]);
					TextOut(hMemDC, 50 + (j*450), 700 + (i*30), str, lstrlen(str));
					wsprintf(str, "Player%d Score: %d", n+1 , User[i].score[j]);
					TextOut(hMemDC, 10 + (j * 450), 10 + (i * 20), str, lstrlen(str));
				}
			}

			for (int i = 0; i < BulletCount; i++)
			{
				//Rectangle(hMemDC, Bullet[i].pos.x, Bullet[i].pos.y, Bullet[i].pos.x + BulletImg[i].GetWidth(), Bullet[i].pos.y + BulletImg[i].GetWidth());
				BulletImg[i].TransparentBlt(hMemDC, Bullet[i].pos.x, Bullet[i].pos.y, BulletImg[i].GetWidth(), BulletImg[i].GetHeight(),
					0, 0,BulletImg[i].GetWidth(), BulletImg[i].GetHeight(), RGB(255,0,255));
			}

			for (int i = 0; i < EnemyCount; i++)
			{
				
				EnemyImg[i].TransparentBlt(hMemDC, Enemy[i].Pos.x, Enemy[i].Pos.y, EnemyImg[i].GetWidth(), EnemyImg[i].GetHeight(),
					0, 0,EnemyImg[i].GetWidth(), EnemyImg[i].GetHeight(), RGB(255,0, 255));
			}
			for (int i = 0; i < ItemCount; i++)
			{
				//Rectangle(hMemDC, Item[i].rt.left, Item[i].rt.top, Item[i].rt.right, Item[i].rt.bottom);

				ItemImg[i].TransparentBlt(hMemDC, Item[i].pos.x, Item[i].pos.y, ItemImg[i].GetWidth(), ItemImg[i].GetHeight(),
					0, 0, ItemImg[i].GetWidth(), ItemImg[i].GetHeight(), RGB(255, 0, 255));
			}
			//wsprintf(str, " X %d", User[0].LIFE);
			//TextOut(hMemDC, 20, 700, str, lstrlen(str));
		}
		/*
		Player[n].TransparentBlt(hMemDC, 20+(j*450), 700, Player[n].GetWidth()/2, Player[n].GetHeight()/2,
		0, 0, Player[n].GetWidth(), Player[n].GetHeight(), RGB(255, 0, 255));
		*/
		

		BitBlt(hdc, 0, 0, ClntRt.right, ClntRt.bottom, hMemDC, 0, 0, SRCCOPY);
		///////////
		DeleteObject(bitmap);
		DeleteDC(hMemDC);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void KeyEventOn()
{
	if (GetAsyncKeyState(VK_LEFT))
	{
		SetEvent(hSendEvent);
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		SetEvent(hSendEvent);
	}
	if (GetAsyncKeyState(VK_UP))
	{
		SetEvent(hSendEvent);
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		SetEvent(hSendEvent);
	}
	if (GetAsyncKeyState('M'))
	{
		SetEvent(hSendEvent);
	}

	if (GetAsyncKeyState(VK_LSHIFT))
	{
		SetEvent(hSendEvent);
	}

	if (GetAsyncKeyState('A'))
	{
		SetEvent(hSendEvent);
	}

	if (GetAsyncKeyState('D'))
	{
		SetEvent(hSendEvent);
	}

	if (GetAsyncKeyState('W'))
	{
		SetEvent(hSendEvent);
	}

	if (GetAsyncKeyState('S'))
	{
		SetEvent(hSendEvent);
	}
}

void Input_Process()
{
	if (GetAsyncKeyState(VK_LEFT))
	{
		ClientBehavior = PLAYER1_LEFT_MOVE;
		
		retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retvalue == SOCKET_ERROR) {
			err_display("send()");
		}
		
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		ClientBehavior = PLAYER1_RIGHT_MOVE;
		
		retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retvalue == SOCKET_ERROR) {
		err_display("send()");
		}
		
	}
	if (GetAsyncKeyState(VK_UP))
	{
		ClientBehavior = PLAYER1_UP_MOVE;
		
		retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retvalue == SOCKET_ERROR) {
		err_display("send()");
		}
		
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		ClientBehavior = PLAYER1_DOWN_MOVE;
		
		retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retvalue == SOCKET_ERROR) {
		err_display("send()");
		}
		
	}
	if (GetAsyncKeyState('M'))
	{
		if (P1Fire)
		{
			ClientBehavior = PLAYER1_FIRE;

			retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
			if (retvalue == SOCKET_ERROR) {
				err_display("send()");
			}
			P1Fire = false;
		}
	}
	
	if (GetAsyncKeyState(VK_LSHIFT))
	{
		if (P2Fire)
		{
			ClientBehavior = PLAYER2_FIRE;

			retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
			if (retvalue == SOCKET_ERROR) {
				err_display("send()");
			}
			P2Fire = false;
		}
	}
	
	if (GetAsyncKeyState('A'))
	{
		ClientBehavior = PLAYER2_LEFT_MOVE;
		
		retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retvalue == SOCKET_ERROR) {
		err_display("send()");
		}
		
	}
	if (GetAsyncKeyState('D'))
	{
		ClientBehavior = PLAYER2_RIGHT_MOVE;
		
		retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retvalue == SOCKET_ERROR) {
		err_display("send()");
		}
		
	}
	if (GetAsyncKeyState('W'))
	{
		ClientBehavior = PLAYER2_UP_MOVE;
		
		retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retvalue == SOCKET_ERROR) {
		err_display("send()");
		}
		
	}
	if (GetAsyncKeyState('S'))
	{
		ClientBehavior = PLAYER2_DOWN_MOVE;
		
		retvalue = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retvalue == SOCKET_ERROR) {
		err_display("send()");
		}
		
	}
}

Plane* CreatePlane()
{
	Plane* NewUnit = new Plane;
	
	return NewUnit;
}

void Init(void)
{
	//각종 정보 초기화 함수
}
Plane* CreateObject(void)
{
	Plane* Obj = new Plane;
	//비행기 생성
	return Obj;
}



//TCP관련
DWORD WINAPI TCPProcess(LPVOID arg)
{
	int retval;

	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	//socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	//connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");
	//printf("[TCP/%s:%d]\n", inet_ntoa(serveraddr.sin_addr), ntohs(serveraddr.sin_port));
	
	CreateThread(NULL, 0, GameDataProcess, NULL, 0, NULL);
	//printf("소켓값 : %d\n", sock);
	//서버와 데이터 통신
	int count = 0;
	while (1)
	{
		WaitForSingleObject(hSendEvent, INFINITE);
		//서버랑 통신하는 부분
		/*
		SHORT값을 서버로 던져준다.
		hSendEvent는 키이벤트가 일어날때마다 풀게 해주면 될듯.
		*/
		Input_Process();
		//혹시 다른 처리가 필요하게 되면 enum값을 범위로 묶고 if else로 구분한다.
		/*
		retval = send(sock, (char*)&ClientBehavior, sizeof(USHORT), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		*/
		//LeaveCriticalSection(&cs);
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}


//TCP관련
DWORD WINAPI GameDataProcess(LPVOID arg)
{
	SOCKADDR_IN clientaddr;
	int addrlen, retval;
	size_t DataLen;
	
	//서버와 데이터 통신
	while (1)
	{
		//타이머에서 주기적으로 갱신.
		WaitForSingleObject(hRecvEvent, INFINITE);
		gamestart = true;//????
		//서버랑 통신하는 부분
		//데이터 길이를 받는 부분
		//gamestart
		
		//총 접속 유저 정보 받음.
		retval = recvn(sock, (char*)&UserCount, sizeof(USHORT), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//데이터 길이 정보 받음.
		retval = recvn(sock, (char*)&DataLen, sizeof(size_t), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		//else if (retval == 0)
		//	break;

		//실제 데이터를 받음.
		retval = recvn(sock, (char*)User, DataLen, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//총 몬스터 숫자 정보
		retval = recvn(sock, (char*)&EnemyCount, sizeof(USHORT), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		//총 몬스터정보길이
		retval = recvn(sock, (char*)&DataLen, sizeof(size_t), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//실제 몬스터데이터
		retval = recvn(sock, (char*)&Enemy, DataLen, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//총 아이템 숫자
		retval = recvn(sock, (char*)&ItemCount, sizeof(USHORT), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//총 아이템정보길이
		retval = recvn(sock, (char*)&DataLen, sizeof(size_t), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//실제 아이템 정보 받음.
		retval = recvn(sock, (char*)&Item, DataLen, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		
		//총 미사일 숫자
		retval = recvn(sock, (char*)&BulletCount, sizeof(USHORT), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//총 미사일정보 길이
		retval = recvn(sock, (char*)&DataLen, sizeof(size_t), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//실제 미사일 정보 받음.
		retval = recvn(sock, (char*)&Bullet, DataLen, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}	
	}

	return 0;
}
