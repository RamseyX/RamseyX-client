#ifndef RX_RAMSEYXTASK_H
#define RX_RAMSEYXTASK_H

#include "Graph.h"
#include "RamseyXHeader.h"
#include "BitsetIterator.h"
#include <mutex>
#include <boost/atomic.hpp>

class RamseyXTask
{
public:
    static std::mutex initMtx;
    static bool firstInstance;

    static int Zp[RX_P]; // to be initialized
    static unsigned int absOfPrimitiveRootPowerTable[RX_MN + 1]; // to be initialized
    static unsigned int absTable[RX_P]; // to be initialized
    static unsigned int q[RX_N + 1]; // to be initialized
    static unsigned long long combinationTable[RX_MN][RX_MN]; // to be initialized

private:
    unsigned int t[RX_N + 1] = {}; // t1 + t2 + t3 = mn = 23
    unsigned int c[RX_N + 1] = {}; // ci = |Ai|
    std::bitset<RX_MN + 1> S[RX_N + 1]; // S[1 ~ 3] values: 1 ~ mn
    std::bitset<RX_MN> B[RX_N + 1]; // B[1 ~ 3] values: 0 ~ mn - 1
    std::bitset<RX_P> A[RX_N + 1]; // A[1 ~ 3] values: 0 ~ p - 1
    Graph GpA[RX_N + 1]; // Gp[Ai]
    Graph GpS[RX_N + 1]; // Gp(Si)
    
    unsigned int a[RX_P] = {};
    unsigned int gen1[RX_P] = {}; // [0 ~ t[1] - 1]
    unsigned int gen2Indices[RX_P] = {};
    unsigned int gen2Source[RX_P] = {}; // [0 ~ t[2] - 1]
    unsigned int gen3[RX_P] = {}; // [0 ~ t[3] - 1]
    bool inB1[RX_MN] = {};
    bool inB3[RX_MN] = {};

    bool restoringFromCheckPoint = false;
    
    bool found = false;
    unsigned long long W1Size = 0;
    unsigned long long blockLength = 0;
    unsigned long long subBlockLength = 0;
    unsigned long long blockBegin = 0;
    unsigned long long blockEnd = 0;
    unsigned long long absOffset = 0;
    unsigned int updateProgressCounter = 0;

    boost::atomic<RXTASKINFO> *infoPtr = nullptr;
    RXTASKINFO cache;
    
public:
    RamseyXTask();
    
	static void init();
	static void initZp();
	static int absOfPrimitiveRootPower(unsigned int exp);
    static int abs(unsigned int x);
    static std::wstring makeSpawnString(const RXTASKINFO &spawnInfo);

    void launch(boost::atomic<RXTASKINFO> &info, const RXFLAG &threadFlag);
    bool generateB1(unsigned int index, const RXFLAG &threadFlag);
    bool generateB1Only(unsigned int index, const RXFLAG &threadFlag);
    bool generateB2(unsigned int index, const RXFLAG &threadFlag);
    bool generateB3();

    void constructSiFromBi(unsigned int i)
    {
        S[i].reset();
        for (unsigned int j = 0; j < RX_MN; ++j)
            if (B[i][j])
                S[i].set(absOfPrimitiveRootPowerTable[j]);
    }
    void constructAiFromSi(unsigned int i)
    {
        int j;
        A[i].reset();
        for (BitsetIterator<RX_MN + 1> iterator(S[i]); (j = iterator.next()) >= 0; )
        {
            A[i].set(indexOfZp(j));
            A[i].set(indexOfZp(-j));
        }
    }
    
    unsigned long long sizeOfW1();
    void calcSizeOfW1(unsigned int index);
    int indexOfZp(int element)
    {
        return element + RX_MN;
    }
    
    void printSQLScript();
};

#endif
