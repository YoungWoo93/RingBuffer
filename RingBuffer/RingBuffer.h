#pragma once
// v.2.0
// single producer / single consumer
//	thread safe version
//


class ringBuffer {
public:
	ringBuffer(const size_t _size = 8192);
	ringBuffer(const ringBuffer& _b);
	ringBuffer& operator =(const ringBuffer& v);

	~ringBuffer();


	inline size_t size() const {
		return (tailPtr - headPtr + bufferSize) % bufferSize;
	}
	inline size_t maxSize() const {
		return bufferSize - 1;
	}
	inline size_t freeSize() const {
		return bufferSize - 1 - (tailPtr - headPtr + bufferSize) % bufferSize;
	}

	inline void clear() {
		headPtr = tailPtr;
	}

	inline bool empty() const {
		return headPtr == tailPtr;
	}
	inline bool full() const {
		return headPtr == buffer + ((tailPtr - buffer + 1) % bufferSize);
	}

	inline char* head()  const {
		return buffer + ((headPtr - buffer + 1) % bufferSize);
	}
	inline char* tail()  const {
		return buffer + ((tailPtr - buffer + 1) % bufferSize);
	}

	inline size_t DirectEnqueueSize() const {
		char* h = headPtr;
		char* t = tailPtr;

		if ((buffer + ((t - buffer + 1) % bufferSize)) <= h)
			return h - (buffer + ((t - buffer + 1) % bufferSize));

		return (buffer + bufferSize) - (buffer + ((t - buffer + 1) % bufferSize));
	}
	inline size_t DirectDequeueSize() const {
		char* h = headPtr;
		char* t = tailPtr;

		if ((buffer + ((h - buffer + 1) % bufferSize)) <= (buffer + ((t - buffer + 1) % bufferSize)))
			return (buffer + ((t - buffer + 1) % bufferSize)) - (buffer + ((h - buffer + 1) % bufferSize));

		return (buffer + bufferSize) - (buffer + ((h - buffer + 1) % bufferSize));
	}



	int MoveRear(const int _size);
	int MoveFront(const int _size);

	size_t	push(const char* _data, size_t _data_size);
	size_t	pop(const char* _buffer, size_t _buffer_size);
	size_t	front(const char* _buffer, size_t _buffer_size) const;

	size_t copy(const ringBuffer& rb);

	/// <summary>
	/// 삭제예정
	/// </summary>
	void printbuffer() const;
	/// 

private:
	size_t	bufferSize;

	char* buffer;
	char* headPtr;
	char* tailPtr;

	char* writePoint;
	char* readPoint;
};













