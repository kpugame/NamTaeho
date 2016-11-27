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
	SingleThread_Processing();//조건1
	MultiThread_Processing();//조건2
	Dekker_Algorithm();//조건3
	Mutex_MultiThread_Processing(); //조건4
	Atomic_MultiThread_Processing();//조건5
	TAS_MultiThread_Processing();//조건6
}

//시간체크
high_resolution_clock::time_point GetNowClock() { return high_resolution_clock::now(); }

void ComputingTime()
{
	cout << "Computing time is " << duration_cast<milliseconds>(ti.end - ti.start).count() << "ms, ";
}

//일반적인 계산
void Normal_Add_Calculate()
{
	for (int i = 0; i < (MAX_NUM / CoreNumber); ++i)
	{
		sum++;
	}
}
//싱글스레드 처리
void SingleThread_Processing()
{
	
	cout << "조건1. 샘플 프로그램 그대로 실행." << endl;
	ti.start = GetNowClock();
	Normal_Add_Calculate();
	ti.end = GetNowClock();
	ComputingTime();
	cout << "Number of Thread = 1, SUM = " << sum << endl;
	sum = 0;
}
//멀티스레드 처리
void MultiThread_Processing()
{
	cout << "\n조건 2. 1, 2, 4, 8, 16개의 스레드로 나누어서 실행 후 각각의 결과\n";
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
//데커의알고리즘1
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
//데커의알고리즘2
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
//데커의 알고리즘 활용
void Dekker_Algorithm()
{
	cout << "\n조건 3. 2개의 스레드로 나누어서 실행한 결과, 데커의 알고리즘으로 상호배제를 구현하시오.\n";

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
//뮤텍스 이용한 계산
void Mutex_Add_Calculate() {
	for (int i = 0; i < (MAX_NUM / CoreNumber); ++i) {
		Mutex_Lock.lock();
		sum += 1;
		Mutex_Lock.unlock();
	}
}
//뮤텍스를 이용한 멀티스레드
void Mutex_MultiThread_Processing()
{
	cout << "\n조건 4. 1, 2, 4, 8, 16개의 스레드로 나누어서 실행 후 각각의 결과 ( mutex 사용 )\n";

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
//아토믹 이용한 계산
void Atomic_Add_Calculate()
{ 
	for (int i = 0; i < (MAX_NUM / CoreNumber); ++i)
		Atomic_sum++;
}
//아토믹를 이용한 멀티스레드
void Atomic_MultiThread_Processing()
{
	cout << "\n조건 5. 1, 2, 4, 8, 16개의 스레드로 나누어서 실행 후 각각의 결과 ( atomic 사용 )\n";

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
//TAS 이용한계산
void TAS_Add_Calculate()
{
	for (int i = 0; i < (MAX_NUM / CoreNumber); ++i) {
		while (atomic_flag_test_and_set(&TAS_Lock));
		sum += 1;
		atomic_flag_clear(&TAS_Lock);
	}
}
//TAS를 이용한 멀티스레드
void TAS_MultiThread_Processing()
{
	cout << "\n조건 6. 1, 2, 4, 8, 16개의 스레드로 나누어서 실행 후 각각의 결과 ( TAS 사용 )\n";

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