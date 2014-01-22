#include "stdafx.h"
#include "Set.h"

Set::Set() : iterator(-1), low(0LL), high(0LL)
{
}

int Set::size() const
{
	int size = 0;
	for (int i = 0; i < 64; i++) {
		if (low & (1LL << i))
			size++;
		if (high & (1LL << i))
			size++;
	}
	return size;
}

void Set::insert(int x)
{
	if (x < 64)
		low |= (1LL << x);
	else
		high |= (1LL << (x - 64));
}

void Set::remove(int x)
{
	if (x < 64)
		low &= ~(1LL << x);
	else
		high &= ~(1LL << (x - 64));
}

bool Set::contain(int x) const
{
	if (x < 64)
		return (low & (1LL << x)) != 0LL;
	else
		return (high & (1LL << (x - 64))) != 0LL;
}

bool Set::isEmpty() const
{
        return (low || high) == 0LL;
}


void Set::resetIterator()
{
	iterator = -1;
}

int Set::next()
{
	for (++iterator; iterator < 128; ++iterator)
		if (iterator < 64) {
			if (low & (1LL << iterator))
				break;
		}
		else if (high & (1LL << (iterator - 64)))
			break;

	return iterator < 128 ? iterator : -1;
}

void Set::clear()
{
	low = high = 0LL;
}

Set Set::operator &(const Set &S) const
{
    Set result;
	result.low = low & S.low;
	result.high = high & S.high;
    
    return result;
}

Set Set::operator |(const Set &S) const
{
    Set result;
	result.low = low | S.low;
	result.high = high | S.high;
    
    return result;
}

void Set::operator -=(const Set &S)
{
	low &= ~S.low;
	high &= ~S.high;
}
