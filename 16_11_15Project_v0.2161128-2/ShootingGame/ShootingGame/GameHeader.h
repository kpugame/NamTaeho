#pragma once
#include<WinSock2.h>
#include<Windows.h>
#include <list>
#include<atlimage.h>
#include"TCPProcess.h"
//Struct
struct UnitInfo // 29 Byte
{
	USHORT Type;// ���� ���� ������, 2 Byte 
	USHORT Action; // ���� �������� ����, 2 Byte
	BYTE HP;// ü��, 1 Byte
	POINT Pos; // ��ġ, 16 Byte
	RECT rt; // �浹 üũ, 16 Byte 
	USHORT AttackDelay;
};

struct Plane
{
	UnitInfo Obj;
	CImage PlaneImg, BulletImg;
};

struct ItemInfo // 12 Byte
{
	USHORT ItemType; // ������ Ÿ��, 2 Byte
	POINT pos; // ������ ��ġ, 8 Byte
	RECT rt;
	//USHORT ItemNum; // ������ ����, 2 Byte
};

struct UserInfo // 86 Byte
{
	SOCKET sock; // ���� ����ü, 16 Byte
	UnitInfo UInfo[2]; // ���� ����, 29 Byte
	//ItemInfo Item[2][3]; // ������ ������, 36 Byte
	USHORT LIFE[2];
	bool Leader, Init; // �������� ����, 1 Byte
	size_t score[2]; // ���� ����, 4 Byte
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
