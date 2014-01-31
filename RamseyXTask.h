#ifndef RX_RAMSEYXTASK_H
#define RX_RAMSEYXTASK_H

#include <atomic>
#include "Graph.h"
#include "RamseyXHeader.h"

#define RX_LAUNCH_INCOMPLETE        -1
#define RX_LAUNCH_SUCCESS           0
#define RX_LAUNCH_REACHED_BLOCK_END 1
#define RX_LAUNCH_INVALID_ARGUMENTS 2

#ifdef UNUSED
#elif defined(__GNUC__)
#define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#else
#define UNUSED(x) x
#endif

struct RXFLAG
{
	std::atomic<bool> *useFlag;
	std::atomic<bool> *termFlag;

	RXFLAG() :
		useFlag(new std::atomic<bool>(false)),
		termFlag(new std::atomic<bool>(false))
	{
	}
    RXFLAG(const RXFLAG &UNUSED(flag)) : // flag unused on intention
		useFlag(new std::atomic<bool>(false)),
		termFlag(new std::atomic<bool>(false))
	{
	}
	~RXFLAG()
	{
		delete useFlag;
		delete termFlag;
	}
};

struct RXTASKINFO
{
	unsigned long long id;
	unsigned int layer;
	unsigned long long combinationNum;
	unsigned long long block;
	unsigned long long offset;
	unsigned long long complexity;
    std::bitset<RX_LAYER1_BLOCKS_PER_TASK> resultBits;
	int result;
	unsigned int threadID;
	std::time_t deadline;
};

class RamseyXTask
{
public:
	static bool firstInstance;

    static int Zp[RX_P]; // to be initialized
    static unsigned int absOfPrimitiveRootPowerTable[RX_MN + 1]; // to be initialized
    static unsigned int absTable[RX_P]; // to be initialized
    static unsigned int q[RX_N + 1]; // to be initialized
    static unsigned long long combinationTable[RX_MN][RX_MN]; // to be initialized

    unsigned int t[RX_N + 1]; // t1 + t2 + t3 = mn = 23
    unsigned int c[RX_N + 1]; // ci = |Ai|
    std::bitset<RX_MN + 1> S[RX_N + 1]; // S[1 ~ 3] values: 1 ~ mn
    std::bitset<RX_MN> B[RX_N + 1]; // B[1 ~ 3] values: 0 ~ mn - 1
    std::bitset<RX_P> A[RX_N + 1]; // A[1 ~ 3] values: 0 ~ p - 1
    Graph GpA[RX_N + 1]; // Gp[Ai]
    Graph GpS[RX_N + 1]; // Gp(Si)
    
    unsigned int a[RX_P];
    unsigned int gen1[RX_P]; // [0 ~ t[1] - 1]
    unsigned int gen2Indices[RX_P];
    unsigned int gen2Source[RX_P]; // [0 ~ t[2] - 1]
    unsigned int gen3[RX_P]; // [0 ~ t[3] - 1]
    bool inB1[RX_MN];
    bool inB3[RX_MN];
    
	bool found;
    unsigned long long W1Size;
    unsigned long long blockLength;
    unsigned long long subBlockLength;
    unsigned long long blockBegin;
    unsigned long long blockEnd;
	unsigned long long absOffset;
    
public:
    RamseyXTask();
    
	static void init();
	static void initZp();
	static int absOfPrimitiveRootPower(unsigned int exp);
    static int abs(unsigned int x);
    static unsigned long long combination(unsigned long long i, unsigned long long j);

	// Multi-thread version
	void t_launch(RXTASKINFO &info, RXFLAG &threadFlag);
    bool t_generateB1(unsigned int index, RXTASKINFO &info, RXFLAG &threadFlag);
    bool t_generateB1Only(unsigned int index, RXTASKINFO &info, RXFLAG &threadFlag);
    bool t_generateB2(unsigned int index, RXTASKINFO &info);
	bool t_generateB3(RXTASKINFO &info);

    void constructSiFromBi(unsigned int i);
    void constructAiFromSi(unsigned int i);
    
    unsigned long long sizeOfW1();
    void calcSizeOfW1(unsigned int index);
    int indexOfZp(int element);
    
    void printSQLScript();
};

#endif
