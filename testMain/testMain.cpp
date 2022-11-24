
#define USE_PROFILE
//#define DEBUG
//#define BUFFER


#ifdef _DEBUG
#pragma comment(lib, "RingBufferD")
#pragma comment(lib, "ProfilerD")

#else
#pragma comment(lib, "RingBuffer")
#pragma comment(lib, "Profiler")

#endif


#include "Profiler/Profiler/Profiler.h"
#include "../RingBuffer/Ringbuffer.h"

#include <iostream>
#include <random>

using namespace std;


string gen_random(const int len) {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;
	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return tmp_s;
}


//#define DEBUG
//#define BUFFER
void allnight_test()
{
	const char* sample = "1234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

	const int BUFFERSIZE = 32;
	int testSize = 1000;
	int testCycle = 1000;

	for (int cc = 0; cc < testCycle; cc++)
	{
		ringBuffer rb(BUFFERSIZE);
		printf("%.2lf\n", static_cast<double>(cc) * 100 / testCycle);
		{
			char temp[BUFFERSIZE];
			string answer;
			SCOPE_PROFILE("cycle test", cf);
			for (int i = 0; i < testSize; i++)
			{
				int start = 0;
				int end = 1 + rand() % 30;



				string input(sample + start, sample + start + end);

#ifdef DEBUG
				cout << "=========================================================" << endl;
				cout << "<< " << input << endl;
				cout << "------------------------------------------------------" << endl;
#endif
				auto pushSize = min(rb.DirectEnqueueSize(), input.length());
				memcpy_s(rb.tail(), rb.DirectEnqueueSize(), input.c_str(), pushSize);
				rb.MoveRear(min(rb.DirectEnqueueSize(), input.length()));
				//auto pushSize = rb.push(input.c_str(), input.length());

				input.erase(input.begin() + pushSize, input.end());
				answer.append(input.c_str());

#ifdef BUFFER
				cout << answer << endl;
				rb.printbuffer();
#endif


				auto popSize = rb.size() / 2;
				popSize = rb.front(temp, popSize);
				popSize = rb.pop(temp, popSize);

				*(temp + popSize) = '\0';
				string test(answer.begin(), answer.begin() + popSize);
				answer.erase(answer.begin(), answer.begin() + popSize);

#ifdef BUFFER
				cout << "Answer : " << answer << endl;
				rb.printbuffer();
#endif

#ifdef DEBUG
				cout << ">> " << temp << endl;
#endif


				if (strcmp(test.c_str(), temp))
				{
					SCOPE_PROFILE(test + " vs " + string(temp), p);
					cout << "fail" << endl;
				}
				else
				{
					SCOPE_PROFILE("success", p);
					cout << "success" << endl;
#ifdef DEBUG
					cout << endl;
#endif
				}

			}
		}
	}




	return;
}


void memcpyVSmemmove()
{
	char src[4096];
	char* dest = (char*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	cout << (intptr_t)(dest) % 4096 << endl;
	int a = 0;
	cin >> a;
	char temp[5];
	memset(src, '_', 4096);
	int maxCycle = 100;
	for (int cycle = 0; cycle < maxCycle; cycle++)
	{
		printf("%d %%\n", cycle);



		for (int i = 1; i < 4096; i++)
		{
			snprintf(temp, sizeof(char) * 5, "%04d", i);
			string name(temp);

			SCOPE_PROFILE("memmove" + name + " byte", p);
			for (int j = 0; j < 10000; j++)
				memmove_s(dest, 4096, src, i);
		}

	}
	for (int cycle = maxCycle; cycle < maxCycle * 2; cycle++)
	{
		printf("%d %%\n", cycle);

		for (int i = 1; i < 4096; i++)
		{
			snprintf(temp, sizeof(char) * 5, "%04d", i);
			string name(temp);

			SCOPE_PROFILE("memmove" + name + " byte", p);
			for (int j = 0; j < 10000; j++)
				memcpy_s(dest, 4096, src, i);
		}
	}

}


void basic_test()
{
	ringBuffer rb(16);
	string s = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char temp[100];

	for (int i = 0; i < 100; i++) {
		rb.printbuffer();
		cout << "DE : " << rb.DirectEnqueueSize() << "\tDD : " << rb.DirectDequeueSize() << endl;
		rb.push(s.c_str(), 27);
		cout << "push " << rb.push(s.c_str(), 27) << endl << endl;

		rb.printbuffer();
		cout << "DE : " << rb.DirectEnqueueSize() << "\tDD : " << rb.DirectDequeueSize() << endl;
		cout << "pop " << rb.pop(temp, 27) << endl << endl;
	}
}


//#include <deque>
#include <queue>
queue<char> pushs;
queue<char> pops;
ringBuffer multiBuffer(32);

SRWLOCK pushLock;
SRWLOCK popLock;
SRWLOCK bufferLock;
unsigned long long pushArr[17] = { 0, };
unsigned long long popArr[17] = { 0, };
queue<int> pushq;
queue<int> popq;
int allpushs = 0;
int allpops = 0;
DWORD WINAPI multi_pusher(LPVOID arg)
{
	unsigned long long cycle = 0;
	unsigned char count = 0;
	multiBuffer.push((char*)&count, 1);
	
	cout << "pusher run" << endl;

	Sleep(1000);
	while (true)
	{
		unsigned char temp[100];
		if (multiBuffer.full())
			continue;

		int inSize = 1 + rand() % multiBuffer.DirectEnqueueSize();
		allpushs += inSize;

		for (int i = 0; i < inSize; i++) {
			temp[i] = ++count;
			pushq.push(temp[i]);
			if (pushq.size() > 10000) {
				queue<int> empty;
				swap(pushq, empty);
			}
			if (count == 0)
				cycle++;
		}
		multiBuffer.push((char*)temp, inSize);
	}
}

DWORD WINAPI multi_poper(LPVOID arg)
{
	unsigned long long cycle = 0;
	unsigned char count = -1;
	cout << "poper run" << endl;


	Sleep(1000);
	while (true)
	{
		unsigned char temp[100];

		if ((allpushs - allpops) == 0)
			continue;

		int outSize = min(rand() % (allpushs - allpops), 100);

		int size = multiBuffer.pop((char*)temp, outSize);

		allpops += size;
		for (int i = 0; i < size; i++) {
			popq.push(temp[i]);
			if (popq.size() > 10000) {
				queue<int> empty;
				swap(popq, empty);
			}

			if (temp[i] == 0)
				cycle++;
			if (++count != temp[i])
				cout << "error here" << endl;
		}
	}
}

void multiThread_test() 
{
	HANDLE hThread = CreateThread(NULL, 0, multi_pusher, NULL, 0, NULL);
	CloseHandle(hThread);
	hThread = CreateThread(NULL, 0, multi_poper, NULL, 0, NULL);
	CloseHandle(hThread);
	double c = 0.0;
	cout.precision(3);
	Sleep(1000);

	while (true) {
		c += 1.0;
	}
}


void main()
{
	//allnight_test();
	multiThread_test();
}