#pragma once
#include<WinSock2.h>
#include<Windows.h>
#include <list>
#include<atlimage.h>
#include"TCPProcess.h"
//Struct
struct UnitInfo // 29 Byte
{
	USHORT Type;// 유저 인지 적인지, 2 Byte 
	USHORT Action; // 무슨 행위인지 저장, 2 Byte
	BYTE HP;// 체력, 1 Byte
	POINT Pos; // 위치, 16 Byte
	RECT rt; // 충돌 체크, 16 Byte 
	USHORT AttackDelay;
};

struct Plane
{
	UnitInfo Obj;
	CImage PlaneImg, BulletImg;
};

struct ItemInfo // 12 Byte
{
	USHORT ItemType; // 아이템 타입, 2 Byte
	POINT pos; // 아이템 위치, 8 Byte
	RECT rt;
	//USHORT ItemNum; // 아이템 개수, 2 Byte
};

struct UserInfo // 86 Byte
{
	SOCKET sock; // 소켓 구조체, 16 Byte
	UnitInfo UInfo[2]; // 유닛 정보, 29 Byte
	//ItemInfo Item[2][3]; // 소유한 아이템, 36 Byte
	USHORT LIFE[2];
	bool Leader, Init; // 방장인지 여부, 1 Byte
	size_t score[2]; // 게임 점수, 4 Byte
};
struct BulletInfo
{
	USHORT Type;
	POINT pos;
	USHORT speed;
	RECT rt;
};

enum OBJTYPE { PLAYER = 1000, ENEMY, MISSILE };
enum UNITACTION { 
	PLAYER1_LEFT_MOVE = 2000, PLAYER1_RIGHT_MOVE, PLAYER1_UP_MOVE, PLAYER1_DOWN_MOVE,
	PLAYER2_LEFT_MOVE = 3000, PLAYER2_RIGHT_MOVE, PLAYER2_UP_MOVE, PLAYER2_DOWN_MOVE,
	PLAYER1_FIRE  = 4000, PLAYER2_FIRE
};

//enumOBJMOVE, FIRE, CREATE, DIE, ITEMUSE };
