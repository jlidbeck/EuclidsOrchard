#pragma once

/*

Implementation of a simple fixed-capacity queue using a circular buffer.

Since the capacity is fixed, no heap allocation is used.

Note that the actual capacity is MAXSIZE-1. This is done to more simply identify the empty state
as head == tail.

Member functions modeled after STL's container classes:

size(): number of elements currently in the queue. 0 <= size <= MAXSIZE-1
capacity(): max allowed number of elements: MAXSIZE-1

push(): adds elements to the tail. If at capacity, returns false and leaves buffer unchanged.
pop(): removes element at head

front(): fetches head element by reference or const reference
back(): fetches tail element by reference or const reference
operator[]: fetches element reference by index, where index==0 is the element at the front (head) of the queue

*/
template <class T, size_t MAXSIZE>
class circular_buffer
{
	T m_v[MAXSIZE];
	int m_tailIndex, m_headIndex;
public:
	// container defs for STL classes
	typedef T value_type;
	typedef size_t size_type;
	typedef T& reference;
	typedef const T& const_reference;

	circular_buffer() {
		m_tailIndex = m_headIndex = 0;
	}

	unsigned int size() const {
		return (m_headIndex <= m_tailIndex
			? m_tailIndex - m_headIndex
			: MAXSIZE - m_headIndex + m_tailIndex);
	}

	inline unsigned int capacity() const {
		return MAXSIZE - 1;
	}

	inline bool empty() const {
		return m_headIndex == m_tailIndex;
	}

	// begin, end {

	inline T& begin() {
		return m_v[m_headIndex];
	}

	inline const T& begin() const {
		return m_v[m_headIndex];
	}

	inline T& end() {
		return m_v[m_tailIndex];
	}

	inline const T& end() const {
		return m_v[m_tailIndex];
	}

	//}

	//	Adds an element to the tail of the queue.
	//	AKA: push_back
	//	If the queue is full (size() == capacity()), returns false and leaves the queue unchanged.
	bool push_back(const T &item) {
		unsigned int nextIndex = m_tailIndex + 1;
		if(nextIndex == MAXSIZE) {
			nextIndex = 0;
		}
		if(nextIndex == m_headIndex) {
			return false;
		}
		m_v[m_tailIndex] = item;
		m_tailIndex = nextIndex;
		return true;
	}


	//	Removes the first element in the queue, that is, the oldest element.
	//	Calling pop() on an empty queue results in undefined behavior.
	void pop() {
		ASSERT(!empty());
		size_t nextIndex = m_headIndex + 1;
		if(nextIndex == MAXSIZE) {
			nextIndex = 0;
		}
		m_headIndex = nextIndex;
	}

	//	Removes the last element in the queue, that is, the most-recently added element.
	//	Calling pop_back() on an empty queue results in undefined behavior.
	void pop_back() {
		ASSERT(!empty());
		ASSERT(m_tailIndex >= 0 && m_tailIndex < MAXSIZE);
		size_t nextIndex = (m_tailIndex ? m_tailIndex - 1 : MAXSIZE - 1);
		m_tailIndex = nextIndex;
	}

	//	Returns a reference to the first element in the queue.
	//	This is the oldest element in the queue.
	//	equivalent to [0]
	//	Calling front() on an empty queue results in undefined behavior.
	const T& front() const {
		ASSERT(!empty());
		return m_v[m_headIndex];
	}

	//	Returns a const reference to the first/head element in the queue, 
	//	leaving the queue's contents unchanged.
	//	Calling front() on an empty queue results in undefined behavior.
	T& front() {
		ASSERT(!empty());
		return m_v[m_headIndex];
	}

	//	Returns a const reference to the last (newest) element in the queue,
	//	leaving the queue's contents unchanged.
	//	Calling back() on an empty queue results in undefined behavior.
	const T& back() const {
		ASSERT(!empty());
		return m_v[m_tailIndex];
	}

	//	Returns a reference to the last (newest) element in the queue,
	//	leaving the queue's contents unchanged.
	//	Calling back() on an empty queue results in undefined behavior.
	T& back() {
		ASSERT(!empty());
		return m_v[m_tailIndex];
	}

	//	Returns reference to element at given index relative to the head of the queue.
	//	this[0] is equivalent to front().
	const T& operator[](unsigned int index) const {
		ASSERT(index >= 0 && index < size());
		index += m_headIndex;
		if(index >= MAXSIZE) {
			index -= MAXSIZE;
			ASSERT(index < m_tailIndex);
		}
		return m_v[index];
	}

	//	Returns reference to element at given index relative to the head of the queue.
	//	this[0] is equivalent to front().
	T& operator[](unsigned int index) {
		ASSERT(index >= 0 && index < size());
		index += m_headIndex;
		if(index >= MAXSIZE) {
			index -= MAXSIZE;
			ASSERT(index < m_tailIndex);
		}
		return m_v[index];
	}

};

