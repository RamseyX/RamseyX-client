#include "constants.h"

class Set
{
private:
	__int64 low;
	__int64 high;
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
};
