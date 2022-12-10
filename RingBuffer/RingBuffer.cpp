// v.2.0
// single producer / single consumer
//	thread safe version
//

#include "Ringbuffer.h"

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>

ringBuffer::ringBuffer(const size_t _size)
	:bufferSize(static_cast<int>(pow(2, ceil(log2(static_cast<double>(_size)))))),
	buffer(new char[bufferSize]),
	headPtr(buffer),
	tailPtr(buffer) {
}

ringBuffer::ringBuffer(const ringBuffer& _b)
	:bufferSize(_b.bufferSize),
	buffer(new char[_b.bufferSize]),
	headPtr(buffer + (_b.headPtr - _b.buffer)),
	tailPtr(buffer + (_b.tailPtr - _b.buffer)) {

	if (headPtr <= tailPtr)
		memmove(head(), _b.head(), _b.size());
	else
	{
		memmove(head(), _b.head(), _b.DirectDequeueSize());
		memmove(buffer, _b.buffer, _b.size() - _b.DirectDequeueSize());
	}
}

ringBuffer& ringBuffer::operator =(const ringBuffer& _b) {
	delete[] buffer;
	bufferSize = _b.bufferSize;
	buffer = new char[_b.bufferSize];
	headPtr = buffer + (_b.headPtr - _b.buffer);
	tailPtr = buffer + (_b.tailPtr - _b.buffer);

	if (headPtr <= tailPtr)
		memmove(head(), _b.head(), _b.size());
	else
	{
		memmove(head(), _b.head(), _b.DirectDequeueSize());
		memmove(buffer, _b.buffer, _b.size() - _b.DirectDequeueSize());
	}

	return *this;
}

ringBuffer::~ringBuffer() {
	delete[] buffer;
}

int ringBuffer::MoveRear(int _size) {
	char* h = headPtr;
	char* t = tailPtr;

	//if (_size > bufferSize - 1 - (t - h + bufferSize) % bufferSize)
	//	_size = bufferSize - 1 - (t - h + bufferSize) % bufferSize;
	if (_size >= 0 && _size > (h - 1 - t + bufferSize) % bufferSize)
		_size = (h - 1 - t + bufferSize) % bufferSize;
	else if (_size < 0 && -_size > (t - h + bufferSize) % bufferSize)
		_size = -(int)((t - h + bufferSize) % bufferSize);

	intptr_t offset = reinterpret_cast<intptr_t>(t) - reinterpret_cast<intptr_t>(buffer);
	offset = (offset + _size + bufferSize) % bufferSize;


	//InterlockedExchange((volatile unsigned long long*)&tailPtr, (unsigned long long)buffer + offset);
	tailPtr = buffer + offset;

	return _size;
}
int ringBuffer::MoveFront(int _size) {
	char* h = headPtr;
	char* t = tailPtr;

	//if (_size > (t - h + bufferSize) % bufferSize)
	//	_size = (t - h + bufferSize) % bufferSize;
	if (_size >= 0 && _size > (t - h + bufferSize) % bufferSize)
		_size = (t - h + bufferSize) % bufferSize;
	else if (_size < 0 && -_size > (h - 1 - t + bufferSize) % bufferSize)
		_size = -(int)((h - 1 - t + bufferSize) % bufferSize);

	intptr_t offset = reinterpret_cast<intptr_t>(h) - reinterpret_cast<intptr_t>(buffer);
	offset = (offset + _size + bufferSize) % bufferSize;

	//InterlockedExchange((volatile unsigned long long*)&headPtr, (unsigned long long)buffer + offset);
	headPtr = buffer + offset;

	return _size;
}

size_t ringBuffer::push(const char* _data, size_t _data_size) {
	pushMutex.lock();

	char* h = headPtr;
	char* t = tailPtr;

	//size_t temp = (bufferSize - 1 - (t - h + bufferSize) % bufferSize);
	size_t temp = (h - 1 - t + bufferSize) % bufferSize;

	if (_data_size > temp)
		_data_size = temp;

	size_t d;
	if ((buffer + ((t - buffer + 1) % bufferSize)) <= h)
		d = h - (buffer + ((t - buffer + 1) % bufferSize));
	else
		d = (buffer + bufferSize) - (buffer + ((t - buffer + 1) % bufferSize));

	if (d >= _data_size)
		memmove(buffer + ((t - buffer + 1) % bufferSize), const_cast<char*>(_data), _data_size);
	else
	{
		memmove(buffer + ((t - buffer + 1) % bufferSize), const_cast<char*>(_data), d);
		memmove(buffer, const_cast<char*>(_data + d), _data_size - d);
	}

	intptr_t offset = reinterpret_cast<intptr_t>(t) - reinterpret_cast<intptr_t>(buffer);
	offset = (offset + _data_size) % bufferSize;

	//InterlockedExchange((volatile unsigned long long*) & tailPtr, (unsigned long long)buffer + offset);
	tailPtr = buffer + offset;

	pushMutex.unlock();
	return _data_size;
}
size_t ringBuffer::pop(const char* _buffer, size_t _buffer_size) {
	popMutex.lock();

	char* h = headPtr;
	char* t = tailPtr;

	size_t temp = (t - h + bufferSize) % bufferSize;
	if (_buffer_size > temp)
		_buffer_size = temp;

	size_t d;
	if ((buffer + ((h - buffer + 1) % bufferSize)) <= (buffer + ((t - buffer + 1) % bufferSize)))
		d = (buffer + ((t - buffer + 1) % bufferSize)) - (buffer + ((h - buffer + 1) % bufferSize));
	else
		d = (buffer + bufferSize) - (buffer + ((h - buffer + 1) % bufferSize));

	if (d >= _buffer_size)
		memmove(const_cast<char*>(_buffer), buffer + ((h - buffer + 1) % bufferSize), _buffer_size);
	else
	{
		memmove(const_cast<char*>(_buffer), buffer + ((h - buffer + 1) % bufferSize), d);
		memmove(const_cast<char*>(_buffer + d), buffer, _buffer_size - d);
	}

	intptr_t offset = reinterpret_cast<intptr_t>(h) - reinterpret_cast<intptr_t>(buffer);
	offset = (offset + _buffer_size) % bufferSize;

	//InterlockedExchange((volatile unsigned long long*) & headPtr, (unsigned long long)buffer + offset);
	headPtr = buffer + offset;

	popMutex.unlock();

	return _buffer_size;
}
size_t ringBuffer::front(const char* _buffer, size_t _buffer_size) {
	popMutex.lock();

	char* h = headPtr;
	char* t = tailPtr;

	size_t temp = (t - h + bufferSize) % bufferSize;
	if (_buffer_size > temp)
		_buffer_size = temp;

	size_t d;
	if ((buffer + ((h - buffer + 1) % bufferSize)) <= (buffer + ((t - buffer + 1) % bufferSize)))
		d = (buffer + ((t - buffer + 1) % bufferSize)) - (buffer + ((h - buffer + 1) % bufferSize));
	else
		d = (buffer + bufferSize) - (buffer + ((h - buffer + 1) % bufferSize));

	if (d >= _buffer_size)
		memmove(const_cast<char*>(_buffer), buffer + ((h - buffer + 1) % bufferSize), _buffer_size);
	else
	{
		memmove(const_cast<char*>(_buffer), buffer + ((h - buffer + 1) % bufferSize), d);
		memmove(const_cast<char*>(_buffer + d), buffer, _buffer_size - d);
	}

	popMutex.unlock();

	return _buffer_size;
}


size_t ringBuffer::copy(const ringBuffer& _b)
{
	headPtr = buffer + (_b.headPtr - _b.buffer);
	tailPtr = buffer + (_b.tailPtr - _b.buffer);

	if (headPtr <= tailPtr)
		memmove(head(), _b.head(), _b.size());
	else
	{
		memmove(head(), _b.head(), _b.DirectDequeueSize());
		memmove(buffer, _b.buffer, _b.size() - _b.DirectDequeueSize());
	}

	return size();
}




void ringBuffer::printbuffer() const
{
	for (int i = 0; i < bufferSize; i++)
		printf("==");
	printf("\n");

	for (int i = 0; i < bufferSize; i++)
	{
		if (buffer + i == head())
			printf(" ->");
		else
			printf("  ");
	}
	printf("\n");

	for (int i = 0; i < bufferSize; i++)
		printf(" %c", *(buffer + i));
	printf("\n");

	for (int i = 0; i < bufferSize; i++)
	{
		if (buffer + i == tail())
			printf("-|");
		else
			printf("  ");
	}
	printf("\n");

	for (int i = 0; i < bufferSize; i++)
		printf("==");
	printf("\n");
}