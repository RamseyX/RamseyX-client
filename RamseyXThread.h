#ifndef RX_RAMSEYXTHREAD_H
#define RX_RAMSEYXTHREAD_H

#include "RamseyXHeader.h"

class RamseyXThread
{
private:
	std::atomic<bool> *termFlag;

public:
	RamseyXThread() :
		termFlag(new std::atomic<bool>(false))
	{
	}

	~RamseyXThread()
	{
		delete termFlag;
	}

	template<class _Fn, class... _Args>
	void run(_Fn&& _Fx, _Args&&... _Ax)
	{
		*termFlag = false;
		std::thread t(_Fx, _Ax);
		t.detach();
	}

	void terminate()
	{
		*termFlag = true;
	}
};

#endif