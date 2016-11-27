#include<iostream>
#include<chrono>
#include<thread>
#include<mutex>
#include<atomic>

using namespace std;
using namespace::chrono;

//Struct
typedef struct _TIMEINFO
{
	high_resolution_clock::time_point start;
	high_resolution_clock::time_point end;
}TIMEINFO;

//Define
#define MAX_NUM 100000000
#define MAX_THREAD_NUM 16

/************************Method*****************/
//time
high_resolution_clock::time_point GetNowClock();
void ComputingTime();
//Calulate
void Normal_Add_Calculate();
void Mutex_Add_Calculate();
void Atomic_Add_Calculate();
void TAS_Add_Calculate();
//ThreadFunc
void SingleThread_Processing();
void MultiThread_Processing();
void Mutex_MultiThread_Processing();
void Atomic_MultiThread_Processing();
void TAS_MultiThread_Processing();
//Dekker's
void Dekker_Algorithm();
void Dekker_Algorithm_Sub1();
void Dekker_Algorithm_Sub2();

/**************Variable********************/
int volatile sum = 0;
int CoreNumber = 1;
TIMEINFO ti;
thread* ThreadArray[MAX_THREAD_NUM];
//Dekker's Algorithm
bool volatile flag[2]{ false, false };
int volatile turn{ 0 };
//mutex
mutex Mutex_Lock;
//atomic
atomic<int> Atomic_sum{ 0 };
//TAS
atomic_flag TAS_Lock = ATOMIC_FLAG_INIT;

int main()
{
	SingleThread_Processing();//����1
	MultiThread_Processing();//����2
	Dekker_Algorithm();//����3
	Mutex_MultiThread_Processing(); //����4
	Atomic_MultiThread_Processing();//����5
	TAS_MultiThread_Processing();//����6
}

//�ð�üũ
high_resolution_clock::time_point GetNowClock() { return high_resolution_clock::now(); }

void ComputingTime()
{
	cout << "Computing time is " << duration_cast<milliseconds>(ti.end - ti.start).count() << "ms, ";
}

//�Ϲ����� ���
void Normal_Add_Calculate()
{
	for (int i = 0; i < (MAX_NUM / CoreNumber); ++i)
	{
		sum++;
	}
}
//�̱۽����� ó��
void SingleThread_Processing()
{
	
	cout << "����1. ���� ���α׷� �״�� ����." << endl;
	ti.start = GetNowClock();
	Normal_Add_Calculate();
	ti.end = GetNowClock();
	ComputingTime();
	cout << "Number of Thread = 1, SUM = " << sum << endl;
	sum = 0;
}
//��Ƽ������ ó��
void MultiThread_Processing()
{
	cout << "\n���� 2. 1, 2, 4, 8, 16���� ������� ����� ���� �� ������ ���\n";
	for (CoreNumber = 1; CoreNumber <= MAX_THREAD_NUM; CoreNumber *= 2)
	{
		for (int i = 0; i < CoreNumber; ++i)
		{
			ThreadArray[i] = new thread{ Normal_Add_Calculate };
		}
		ti.start = GetNowClock();
		for (int i = 0; i < CoreNumber; ++i)
		{
			ThreadArray[i]->join();
			delete ThreadArray[i];
		}
		ti.end = GetNowClock();
		ComputingTime();
		cout << "Number of Thread = " << CoreNumber << ",\tSUM = " << sum << endl;
		sum = 0;
	}
}
//��Ŀ�Ǿ˰���1
void Dekker_Algorithm_Sub1() {
	for (int i = 0; i < MAX_NUM / 2; ++i)
	{
		flag[0] = true;
		while (flag[1] == true) {
			if (turn == 1) {
				flag[0] = false;
				while (turn == 1);
				flag[0] = true;
			}
		}
		++sum;

		turn = 1;
		flag[0] = false;
	}
}
//��Ŀ�Ǿ˰���2
void  Dekker_Algorithm_Sub2() {
	for (int i = 0; i < MAX_NUM / 2; ++i)
	{
		flag[1] = true;
		while (flag[0] == true) {
			if (turn == 0) {
				flag[1] = false;
				while (turn == 0);
				flag[1] = true;
			}
		}
		++sum;

		turn = 0;
		flag[1] = false;
	}
}
//��Ŀ�� �˰��� Ȱ��
void Dekker_Algorithm()
{
	cout << "\n���� 3. 2���� ������� ����� ������ ���, ��Ŀ�� �˰������� ��ȣ������ �����Ͻÿ�.\n";

	thread FirstThread{ Dekker_Algorithm_Sub1 };
	thread SecondThread{ Dekker_Algorithm_Sub2 };

	ti.start = GetNowClock();
	FirstThread.join();
	SecondThread.join();
	ti.end = GetNowClock();
	ComputingTime();
	cout << "Number of Thread = 2,\tSUM = " << sum << endl;
	sum = 0;

}
//���ؽ� �̿��� ���
void Mutex_Add_Calculate() {
	for (int i = 0; i < (MAX_NUM / CoreNumber); ++i) {
		Mutex_Lock.lock();
		sum += 1;
		Mutex_Lock.unlock();
	}
}
//���ؽ��� �̿��� ��Ƽ������
void Mutex_MultiThread_Processing()
{
	cout << "\n���� 4. 1, 2, 4, 8, 16���� ������� ����� ���� �� ������ ��� ( mutex ��� )\n";

	for (CoreNumber = 1; CoreNumber <= MAX_THREAD_NUM; CoreNumber *= 2)
	{
		for (int i = 0; i < CoreNumber; ++i)
		{
			ThreadArray[i] = new thread{ Mutex_Add_Calculate };
		}
		ti.start = GetNowClock();
		for (int i = 0; i < CoreNumber; ++i)
		{
			ThreadArray[i]->join();
			delete ThreadArray[i];
		}
		ti.end = GetNowClock();
		ComputingTime();
		cout << "Number of Thread = " << CoreNumber << ",\tSUM = " << sum << endl;
		sum = 0;
	}
}
//����� �̿��� ���
void Atomic_Add_Calculate()
{ 
	for (int i = 0; i < (MAX_NUM / CoreNumber); ++i)
		Atomic_sum++;
}
//����͸� �̿��� ��Ƽ������
void Atomic_MultiThread_Processing()
{
	cout << "\n���� 5. 1, 2, 4, 8, 16���� ������� ����� ���� �� ������ ��� ( atomic ��� )\n";

	for (CoreNumber = 1; CoreNumber <= MAX_THREAD_NUM; CoreNumber *= 2)
	{
		for (int i = 0; i < CoreNumber; ++i)
		{
			ThreadArray[i] = new thread{ Atomic_Add_Calculate };
		}
		ti.start = GetNowClock();
		for (int i = 0; i < CoreNumber; ++i)
		{
			ThreadArray[i]->join();
			delete ThreadArray[i];
		}
		ti.end = GetNowClock();
		ComputingTime();
		cout << "Number of Thread = " << CoreNumber << ",\tSUM = " << Atomic_sum << endl;
		Atomic_sum = 0;
	}
}
//TAS �̿��Ѱ��
void TAS_Add_Calculate()
{
	for (int i = 0; i < (MAX_NUM / CoreNumber); ++i) {
		while (atomic_flag_test_and_set(&TAS_Lock));
		sum += 1;
		atomic_flag_clear(&TAS_Lock);
	}
}
//TAS�� �̿��� ��Ƽ������
void TAS_MultiThread_Processing()
{
	cout << "\n���� 6. 1, 2, 4, 8, 16���� ������� ����� ���� �� ������ ��� ( TAS ��� )\n";

	for (CoreNumber = 1; CoreNumber <= MAX_THREAD_NUM; CoreNumber *= 2)
	{
		for (int i = 0; i < CoreNumber; ++i)
		{
			ThreadArray[i] = new thread{ TAS_Add_Calculate };
		}
		ti.start = GetNowClock();
		for (int i = 0; i < CoreNumber; ++i)
		{
			ThreadArray[i]->join();
			delete ThreadArray[i];
		}
		ti.end = GetNowClock();
		ComputingTime();
		cout << "Number of Thread = " << CoreNumber << ",\tSUM = " << sum << endl;
		sum = 0;
	}
}