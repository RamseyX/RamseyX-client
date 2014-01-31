#ifndef RX_SET_H
#define RX_SET_H

#include <bitset>

template<std::size_t T>
class BitsetIterator
{
private:
	int iterator;
	const std::bitset<T> *bitvec;

public:
	BitsetIterator(const std::bitset<T> &bits);
	void rebind(const std::bitset<T> &bits);
	void reset();
	int next();
};

template<std::size_t T>
BitsetIterator<T>::BitsetIterator(const std::bitset<T> &bits) : iterator(-1), bitvec(&bits)
{
}

template<std::size_t T>
void BitsetIterator<T>::rebind(const std::bitset<T> &bits)
{
	bitvec = &bits;
	iterator = -1;
}

template<std::size_t T>
void BitsetIterator<T>::reset()
{
	iterator = -1;
}

template<std::size_t T>
int BitsetIterator<T>::next()
{
    for (++iterator; iterator < T && !bitvec->operator[](iterator); ++iterator);

	return iterator < T ? iterator : -1;
}

/*class Set
{
private:
	long long low;
	long long high;
    int iterator;
    
public:
	Set();
	int size() const;
	void insert(int x);
	void remove(int x);
	bool contain(int x) const;
	bool isEmpty() const;
	void resetIterator();
	int next();
	void clear();
    Set operator &(const Set &S) const;
    Set operator |(const Set &S) const;
    void operator -=(const Set &S);
};*/

#endif
