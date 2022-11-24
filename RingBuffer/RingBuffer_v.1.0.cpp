// v.1.0
//	single thread version
//

#include "Ringbuffer.h"

#include <string.h>
#include <math.h>
#include <stdio.h>

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

size_t ringBuffer::MoveRear(size_t _size) {
	if (_size > freeSize())
		_size = freeSize();

	intptr_t offset = reinterpret_cast<intptr_t>(tailPtr) - reinterpret_cast<intptr_t>(buffer);
	offset = (offset + _size) % bufferSize;

	tailPtr = buffer + offset;

	return _size;
}
size_t ringBuffer::MoveFront(size_t _size) {
	if (_size > size())
		_size = size();

	intptr_t offset = reinterpret_cast<intptr_t>(headPtr) - reinterpret_cast<intptr_t>(buffer);
	offset = (offset + _size) % bufferSize;

	headPtr = buffer + offset;

	return _size;
}

size_t ringBuffer::push(const char* _data, size_t _data_size) {
	if (_data_size > freeSize())
		_data_size = freeSize();

	size_t d = DirectEnqueueSize();


	if (d >= _data_size)
		memmove(tail(), const_cast<char*>(_data), _data_size);
	else
	{
		memmove(tail(), const_cast<char*>(_data), d);
		memmove(buffer, const_cast<char*>(_data + d), _data_size - d);
	}

	MoveRear(_data_size);

	return _data_size;
}
size_t ringBuffer::pop(const char* _buffer, size_t _buffer_size) {
	if (_buffer_size > size())
		_buffer_size = size();

	size_t d = DirectDequeueSize();

	if (headPtr == tailPtr)
		d = 0;
	else if (headPtr <= tailPtr)
		d = tailPtr - headPtr;

	if (d >= _buffer_size)
		memmove(const_cast<char*>(_buffer), head(), _buffer_size);
	else
	{
		memmove(const_cast<char*>(_buffer), head(), d);
		memmove(const_cast<char*>(_buffer + d), buffer, _buffer_size - d);
	}

	MoveFront(_buffer_size);

	return _buffer_size;
}
size_t ringBuffer::front(const char* _buffer, size_t _buffer_size)  const {
	if (_buffer_size > size())
		_buffer_size = size();

	size_t d = DirectDequeueSize();

	if (d >= _buffer_size)
		memmove(const_cast<char*>(_buffer), head(), _buffer_size);
	else
	{
		memmove(const_cast<char*>(_buffer), head(), d);
		memmove(const_cast<char*>(_buffer + d), buffer, _buffer_size - d);
	}

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