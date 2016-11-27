#include<iostream>
#include<thread>
#include<mutex>
#include<atomic>
#include<chrono>
using namespace std;
using namespace::chrono;

#define MAX 10000

//Struct
typedef struct _TIMEINFO
{
	high_resolution_clock::time_point start;
	high_resolution_clock::time_point end;
	void (*pf)(void);
}TIMEINFO;

//class
class Fork
{
private:
	atomic<bool> state{ false };
	mutex lock;
public:
	void release() {
		state = false;
		lock.unlock();
	}
	bool getfork() {
		if (true == state)
			return false;
		lock.lock();
		state = true;
		return true;
	}
};

//Variable
Fork forks[5];
thread* threadArray[5];
TIMEINFO timeInfo;

//Method
void Philosophers(const int id);
void Odd_Philosophers(const int id);
int GetLeftFork(const int id);
int GetRightFork(const int id);
high_resolution_clock::time_point GetNowClock();
void ComputingTime();

int main()
{
	/*
	비대칭 해결법을 사용.
	홀수번째 철학자는 왼쪽 포크를 집은 후 오른쪽 포크를 집게하고
	짝수번째 철학자는 오른쪽 포크를 집은 후 왼쪽 포크를 집게한다.
	*/
	timeInfo.pf = ComputingTime;
	cout << "Dining Philosophers Problem Program\n";
	for (int i = 0; i < 5; ++i)
	{
		if (i % 2 == 0)
			threadArray[i] = new thread{ Philosophers, i };
		else
			threadArray[i] = new thread{ Odd_Philosophers, i };
		cout << i << "th Philosophers is Ready to Eat.\n";
	}
	timeInfo.start = GetNowClock();

	cout << "\nDuring the Meal...\n\n";

	for (int i = 0; i < 5; ++i)
	{
		threadArray[i]->join(); delete threadArray[i];
	}
	timeInfo.end = GetNowClock();
	timeInfo.pf();
}

void Philosophers(const int id)
{
	int left, right;

	for (int i = 0; i < MAX; ++i)
	{
		right = GetRightFork(id);
		left = GetLeftFork(id);

		forks[right].release(); forks[left].release();
		printf("%d %d", left, right);
	}
	cout << id << "th Philosophers finish the Meal.\n";
}
void Odd_Philosophers(const int id)
{
	int left, right;
	for (int i = 0; i < MAX; ++i)
	{
		left = GetLeftFork(id);
		right = GetRightFork(id);

		forks[left].release(); forks[right].release();
		printf("%d %d", left, right);
	}
	cout << id << "th Philosophers finish the Meal.\n";

}

int GetLeftFork(const int id)
{
	while (1)
	{
		if (forks[id].getfork())
			break;
	}
	return id;
}

int GetRightFork(const int id)
{
	while (1)
	{
		if (id < 4) {
			if (forks[id + 1].getfork())
				break;
		}
		else if (id == 4) {
			if (forks[0].getfork())
				break;
		}
	}
	return id < 4 ? id + 1 : 0;
}

//시간체크
high_resolution_clock::time_point GetNowClock() { return high_resolution_clock::now(); }

void ComputingTime()
{
	cout << "Computing time is " << duration_cast<milliseconds>(timeInfo.end - timeInfo.start).count() << "ms\n";
}