/***************************************************************************
 *
 * RamseyX Client: client program of distributed computing project RamseyX
 *
 * Copyright (C) 2013-2014 Zizheng Tai <zizheng.tai@gmail.com>, et al.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/
#ifndef RAMSEYXTASK_H
#define RAMSEYXTASK_H

#include "graph.h"
#include "ramseyxdefs.h"
#include "bitsetiterator.h"
#include <ctime>
#include <mutex>
// gcc 4.8.2 still has not fully supported std::atomic
#ifdef _MSC_VER
#include <atomic>
namespace boost = std;
#else
#include <boost/atomic.hpp>
#endif

struct RXTASKINFO
{
    unsigned long long id = 0;
    unsigned int layer = 0;
    std::time_t deadline = 0;
    unsigned long long combination = 0;
    unsigned long long block = 0;
    unsigned long long offset = 0;
    unsigned long long complexity = 0;
    int result = RX_LAUNCH_INCOMPLETE;
    unsigned int threadID = 0;

    std::bitset<RX_LAYER1_BLOCKS_PER_TASK> resultBits;
};

struct RXFLAG
{
    std::atomic<bool> useFlag{false};
    std::atomic<bool> termFlag{false};
};

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
    static std::string makeSpawnString(const RXTASKINFO &spawnInfo);

    void launch(boost::atomic<RXTASKINFO> &info, const RXFLAG &threadFlag);
    bool generateB1(unsigned int index, const RXFLAG &threadFlag);
    bool generateB1Only(unsigned int index, const RXFLAG &threadFlag);
    bool generateB2(unsigned int index, const RXFLAG &threadFlag);
    bool generateB3();

    inline void constructSiFromBi(unsigned int i);
    inline void constructAiFromSi(unsigned int i);

    unsigned long long sizeOfW1();
    void calcSizeOfW1(unsigned int index);
    inline int indexOfZp(int element);

    void printSQLScript();
};

bool operator==(const RXTASKINFO &lhs, const RXTASKINFO &rhs);
std::ofstream &operator<<(std::ofstream &lhs, const RXTASKINFO &rhs);
std::ifstream &operator>>(std::ifstream &lhs, RXTASKINFO &rhs);

#endif // RAMSEYXTASK_H
