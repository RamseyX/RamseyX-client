/***************************************************************************
 *
 * RamseyX Client: client program of distributed computing project RamseyX
 *
 * Copyright (C) 2013-2014 Zizheng Tai <zizheng.tai@gmail.com>
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
#include "ramseyxtask.h"
#include "bitsetiterator.h"
#include <cstring>
#include <fstream>
#include <memory>

std::mutex RamseyXTask::initMtx;
bool RamseyXTask::firstInstance = true;
int RamseyXTask::Zp[RX_P] = {};
unsigned int RamseyXTask::absOfPrimitiveRootPowerTable[RX_MN + 1] = {};
unsigned int RamseyXTask::absTable[RX_P] = {};
unsigned int RamseyXTask::q[RX_N + 1] = {};
unsigned long long RamseyXTask::combinationTable[RX_MN][RX_MN] = {};

RamseyXTask::RamseyXTask()
{
    std::lock_guard<std::mutex> lk(initMtx);
    if (firstInstance)
    {
        init();
        firstInstance = false;
    }
    std::memset(t, 0, sizeof (t));
    std::memset(c, 0, sizeof (c));
    std::memset(a, 0, sizeof (a));
    std::memset(gen1, 0, sizeof (gen1));
    std::memset(gen2Indices, 0, sizeof (gen2Indices));
    std::memset(gen2Source, 0, sizeof (gen2Source));
    std::memset(gen3, 0, sizeof (gen3));
    std::memset(inB1, 0, sizeof (inB1));
    std::memset(inB3, 0, sizeof (inB3));
}

void RamseyXTask::init()
{
    q[1] = RX_Q1;
    q[2] = RX_Q2;
    q[3] = RX_Q3;
    initZp();
    for (unsigned int i = 0; i <= RX_MN; ++i)
        absOfPrimitiveRootPowerTable[i] = absOfPrimitiveRootPower(i);
    for (unsigned int i = 0; i < RX_P; ++i)
        absTable[i] = abs(i);
    for (unsigned int i = 1; i < RX_MN; ++i)
    {
        combinationTable[i][1] = i;
        combinationTable[i][i] = combinationTable[i][0] = 1;
    }
    for (unsigned int i = 1; i < RX_MN; ++i)
        for (unsigned int j = 1; j < i; ++j)
            combinationTable[i][j] = combinationTable[i - 1][j - 1] + combinationTable[i - 1][j];
}

int RamseyXTask::abs(unsigned int x)
{
    x %= RX_P;
    return RX_P < (x << 1) ? RX_P - x : x;
}

void RamseyXTask::launch(boost::atomic<RXTASKINFO> &info, const RXFLAG &threadFlag)
{
    infoPtr = &info;
    cache = info;

    // Make sure that info is updated before leaving the function
    typedef std::pair<boost::atomic<RXTASKINFO> *, RXTASKINFO *> InfoPair;
    auto deleter = [](InfoPair *p) {
        p->first->store(*(p->second));
        delete p;
    };
    std::unique_ptr<InfoPair, decltype(deleter)>
            p(new InfoPair(&info, &cache), deleter);

    // Layer 1
    if (cache.layer == 1)
    {
        // When a new task is added, cache.combinationNum and cache.block indicate
        // the beginning block (inclusive). Afterwards, they indicate the block
        // begin currently processed.
        // A layer-1 task will span LAYER1_BLOCKS_PER_TASK blocks unless the number of
        // remaining blocks is less than LAYER1_BLOCKS_PER_TASK.
        
        unsigned long long currentComb = 0, currentBlock = 0, currentOffset = 0;
        for (t[1] = q[1] + 1; RX_MN - t[1] >= q[2] + q[3] + 2; ++t[1])
            for (t[2] = q[2] + 1; RX_MN - t[1] - t[2] >= q[3] + 1;
                ++t[2], ++currentComb)
            {
                if (cache.offset >= RX_LAYER1_BLOCKS_PER_TASK)
                {
                    cache.result = RX_LAUNCH_REACHED_BLOCK_END;
                    return;
                }

                if (currentComb < cache.combination)
                    continue;
                // Now currentComb == cache.combinationNum
                t[3] = RX_MN - t[1] - t[2];
                if (t[3] < q[3] + 1)
                {
                    cache.result = RX_LAUNCH_REACHED_BLOCK_END;
                    return;
                }

                subBlockLength = combinationTable[RX_MN - t[1]][t[2]];
                blockLength = taskInfo[currentComb][RX_TASKINFO_BLOCKLENGTH];
                W1Size = taskInfo[currentComb][RX_TASKINFO_W1SIZE];

                if (currentComb == cache.combination)
                    currentBlock = cache.block;
                else
                    currentBlock = 0;

                restoringFromCheckPoint = false; // Check point valid only within the same combination
                absOffset = 0;
                gen1[1] = 1;

                for (; currentBlock < taskInfo[currentComb][RX_TASKINFO_BLOCKNUM] &&
                     cache.offset < RX_LAYER1_BLOCKS_PER_TASK;
                     ++currentBlock, ++currentOffset, ++cache.offset)
                {
                    if (currentOffset < cache.offset)
                    {
                        --cache.offset;
                        continue;
                    }

                    // [blockBegin, blockEnd)
                    blockBegin = currentBlock * blockLength;
                    if (W1Size < blockBegin + blockLength)
                    {
                        blockEnd = W1Size;
                        blockLength = blockEnd - blockBegin;
                    }
                    else
                        blockEnd = blockBegin + blockLength;
                    if (blockBegin >= blockEnd)
                        break; // Next combination

                    found = false;

                    for (/*gen1[1] = 1*/; gen1[1] * (t[1] - 1) < RX_MN; ++gen1[1])
                        if (!generateB1Only(2, threadFlag))
                            break;

                    if (threadFlag.termFlag)
                        return;
                    else if (found)
                        cache.resultBits.set(static_cast<std::size_t>(cache.offset));
                    else
                        cache.resultBits.reset(static_cast<std::size_t>(cache.offset));
                }
            }
        cache.result = RX_LAUNCH_REACHED_BLOCK_END;
        return;
    }

    // Layer 3
    unsigned long long i = 0;
    for (t[1] = q[1] + 1; RX_MN - t[1] >= q[2] + q[3] + 2; ++t[1])
        for (t[2] = q[2] + 1; RX_MN - t[1] - t[2] >= q[3] + 1; ++t[2], ++i)
            if (i == cache.combination)
                goto COMB_FOUND;
COMB_FOUND:
    t[3] = RX_MN - t[1] - t[2];
    if (t[3] < q[3] + 1)
    {
        cache.result = RX_LAUNCH_INVALID_ARGUMENTS;
        return;
    }

    subBlockLength = combinationTable[RX_MN - t[1]][t[2]];
    blockLength = taskInfo[cache.combination][RX_TASKINFO_BLOCKLENGTH];
    W1Size = taskInfo[cache.combination][RX_TASKINFO_W1SIZE];
    blockBegin = cache.block * blockLength;
    if (W1Size < blockBegin + blockLength)
    {
        blockEnd = W1Size;
        blockLength = blockEnd - blockBegin;
    }
    else
        blockEnd = blockBegin + blockLength;
    if (blockBegin >= blockEnd)
    {
        cache.result = RX_LAUNCH_INVALID_ARGUMENTS;
        return;
    }

    found = false;
    absOffset = 0;
    updateProgressCounter = 0;

    // generateB1(1)
    for (gen1[1] = 1; gen1[1] * (t[1] - 1) < RX_MN; ++gen1[1])
        if (!generateB1(2, threadFlag))
            break;

    if (threadFlag.termFlag)
        return;
    else if (found)
        cache.result = RX_LAUNCH_SUCCESS;
    else if (absOffset >= blockEnd)
        cache.result = RX_LAUNCH_REACHED_BLOCK_END;
}

bool RamseyXTask::generateB1(unsigned int index, const RXFLAG &threadFlag)
{
    if (index == t[1])
    {
        if (threadFlag.termFlag)
            return false;
        if (updateProgressCounter > RX_UPDATE_PROGRESS_COUNT) // Layer 3
        {
            updateProgressCounter = 0;
            infoPtr->store(cache);
        }

        // S1 done; S2 to be generated
        if (absOffset + subBlockLength <= blockBegin + cache.offset)
        {
            absOffset += subBlockLength;
            return true;
        }

        ++cache.complexity;
        ++updateProgressCounter;

        unsigned int i, j, k;
        // validate B[1] (GpS[1])
        B[1].reset();
        for (i = 0; i < t[1]; ++i)
            B[1].set(gen1[i]);
        constructSiFromBi(1);
        constructAiFromSi(1);

        GpA[1].clear();
        for (j = 0; j < RX_P; ++j)
            if (A[1][j])
                for (k = j + 1; k < RX_P; ++k)
                    if (A[1][k] && S[1][absTable[Zp[k] - Zp[j]]])
                        GpA[1].connect(j, k);
        if (GpA[1].cliqueExists<RX_Q1 - 1>()/*size2CliqueExists()*/)
        {
            absOffset += subBlockLength;
            cache.offset += subBlockLength;
            return absOffset < blockEnd;
        }

        // prepare for B[2]
        std::memset(inB1, false, sizeof (inB1));
        for (i = 0; i < t[1]; ++i)
            inB1[gen1[i]] = true;
        for (i = 1, j = 0; i < RX_MN; ++i)
            if (!inB1[i])
                gen2Source[j++] = i;

        // generateB2(1)
        for (gen2Indices[0] = 0; gen2Indices[0] < t[2] + t[3]; ++gen2Indices[0])
            if (!generateB2(1, threadFlag))
                return false;

        absOffset += subBlockLength;
        cache.offset += subBlockLength;
        return absOffset < blockEnd;
    }
    for (gen1[index] = gen1[index - 1] + gen1[1];
        gen1[index] + gen1[1] * (t[1] - 1 - index) < RX_MN;
        ++gen1[index])
        if (!generateB1(index + 1, threadFlag))
            return false;
    
    return true;
}

bool RamseyXTask::generateB1Only(unsigned int index, const RXFLAG &threadFlag)
{
    if (index == t[1])
    {
        if (!(cache.complexity & 15) && threadFlag.termFlag)
            return false;
        if (absOffset + subBlockLength <= blockBegin)
        {
            absOffset += subBlockLength;
            return true;
        }

        restoringFromCheckPoint = true;

        if (!(cache.complexity & 65535)) // Layer 1
            infoPtr->store(cache);

        ++cache.complexity;

        unsigned int i, j, k;
        // validate B[1] (GpS[1])
        B[1].reset();
        for (i = 0; i < t[1]; ++i)
            B[1].set(gen1[i]);
        constructSiFromBi(1);
        constructAiFromSi(1);

        GpA[1].clear();
        for (j = 0; j < RX_P; ++j)
            if (A[1][j])
                for (k = j + 1; k < RX_P; ++k)
                    if (A[1][k] && S[1][absTable[Zp[k] - Zp[j]]])
                        GpA[1].connect(j, k);
        if (GpA[1].cliqueExists<RX_Q1 - 1>()/*size2CliqueExists()*/)
        {
            absOffset += subBlockLength;
            if (absOffset < blockEnd)
                return true;
            else
            {
                absOffset -= subBlockLength;
                return false;
            }
        }
        
        // Qualified S[1] found
        found = true;
        return false;
    }
    if (!restoringFromCheckPoint)
        gen1[index] = gen1[index - 1] + gen1[1];
    for (/*gen1[index] = gen1[index - 1] + gen1[1]*/;
        gen1[index] + gen1[1] * (t[1] - 1 - index) < RX_MN;
        ++gen1[index])
        if ((restoringFromCheckPoint = !generateB1Only(index + 1, threadFlag))) // Assignment on purpose
            return false;

    return true;
}

bool RamseyXTask::generateB2(unsigned int index, const RXFLAG &threadFlag)
{
    if (index == t[2])
    {
        if (!(cache.complexity & 15) && threadFlag.termFlag)
            return false;

        ++cache.complexity;
        ++updateProgressCounter;

        //validate B[2] (GpS[2])
        unsigned int i, j, k;
        B[2].reset();
        for (i = 0; i < t[2]; ++i)
            B[2].set(gen2Source[gen2Indices[i]]);
        constructSiFromBi(2);
        constructAiFromSi(2);

        GpA[2].clear();
        for (j = 0; j < RX_P; ++j)
            if (A[2][j])
                for (k = j + 1; k < RX_P; ++k)
                    if (A[2][k] && S[2][absTable[Zp[k] - Zp[j]]])
                        GpA[2].connect(j, k);
        if (GpA[2].cliqueExists<RX_Q2 - 1>()/*size2CliqueExists()*/)
            return true;

        // S2 done; S3 to be generated
        std::memset(inB3, true, sizeof (inB3));
        for (i = 0; i < t[1]; ++i)
            inB3[gen1[i]] = false;
        for (i = 0; i < t[2]; ++i)
            inB3[gen2Source[gen2Indices[i]]] = false;

        return generateB3();
    }
    for (gen2Indices[index] = gen2Indices[index - 1] + 1;
        gen2Indices[index] < t[2] + t[3];
        ++gen2Indices[index])
        if (!generateB2(index + 1, threadFlag))
            return false;

    return true;
}

bool RamseyXTask::generateB3()
{
    ++cache.complexity;
    ++updateProgressCounter;

    unsigned int i, j, k;
    for (i = 1, j = 0; i < RX_MN; ++i)
    if (inB3[i])
        gen3[j++] = i;

    // validate B[3]
    B[3].reset();
    for (i = 0; i < t[3]; ++i)
        B[3].set(gen3[i]);
    constructSiFromBi(3);
    constructAiFromSi(3);

    GpA[3].clear();
    for (j = 0; j < RX_P; ++j)
        if (A[3][j])
            for (k = j + 1; k < RX_P; ++k)
                if (A[3][k] && S[3][absTable[Zp[k] - Zp[j]]])
                    GpA[3].connect(j, k);
    if (GpA[3].cliqueExists<RX_Q3 - 1>()/*size5CliqueExists()*/)
        return true;

    // found!
    found = true;
    return false;
}

void RamseyXTask::constructSiFromBi(unsigned int i)
{
    S[i].reset();
    for (unsigned int j = 0; j < RX_MN; ++j)
        if (B[i][j])
            S[i].set(absOfPrimitiveRootPowerTable[j]);
}

void RamseyXTask::constructAiFromSi(unsigned int i)
{
    int j;
    A[i].reset();
    for (BitsetIterator<RX_MN + 1> iterator(S[i]); (j = iterator.next()) >= 0; )
    {
        A[i].set(indexOfZp(j));
        A[i].set(indexOfZp(-j));
    }
}

void RamseyXTask::calcSizeOfW1(unsigned int index)
{
    if (index == t[1] - 1)
        W1Size += RX_MN -  a[index - 1] - a[1];
    else
        for (a[index] = a[index - 1] + a[1]; a[index] + a[1] * (t[1] - 1 - index) < RX_MN; ++a[index])
            calcSizeOfW1(index + 1);
}

unsigned long long RamseyXTask::sizeOfW1()
{
    W1Size = 0;
    for (a[1] = 1; a[1] * (t[1] - 1) < RX_MN; ++a[1])
        calcSizeOfW1(2);
    W1Size *= combinationTable[RX_MN - t[1]][t[2]];
    
    return W1Size;
}

int RamseyXTask::absOfPrimitiveRootPower(unsigned int exp)
{
    // |g^exp|
    unsigned int result = 1;
    for (unsigned int i = 0; i < exp; ++i)
    {
        result *= RX_G;
        result %= RX_P;
    }
    
    return RX_P < (result << 1) ? RX_P - result : result;
}

void RamseyXTask::initZp()
{
    for (unsigned int i = 0; i < RX_P; ++i)
        Zp[i] = static_cast<int>(i) - static_cast<int>(RX_MN);
}

std::string RamseyXTask::makeSpawnString(const RXTASKINFO &spawnInfo)
{
    static const unsigned long long totalCombNum = sizeof (taskInfo) / sizeof (taskInfo[0]);

    std::string result(std::to_string(spawnInfo.resultBits.count()));

    unsigned long long comb = spawnInfo.combination, block = spawnInfo.block;
    for (unsigned int i = 0; i < RX_LAYER1_BLOCKS_PER_TASK; ++i, ++block)
    {
        if (block == taskInfo[comb][RX_TASKINFO_BLOCKNUM])
        {
            block = 0;
            ++comb;
        }
        if (comb == totalCombNum)
            break;
        if (spawnInfo.resultBits[i])
        {
            result += ',';
            result += std::to_string(comb);
            result += ',';
            result += std::to_string(block);
        }
    }

    return result;
}

int RamseyXTask::indexOfZp(int element)
{
    return element + RX_MN;
}

void RamseyXTask::printSQLScript()
{
    std::ofstream ftaskinfo("task_info.txt");
    std::ofstream ftasks("task.txt");
    std::ofstream farray("array.txt");
    farray << "unsigned long long taskInfo[][3] = {\n";
    char sz[200] = {};
    unsigned long long currentCombination = 0;
    unsigned long long layer1TaskNum = 0, layer3TaskNum = 0;
    for (t[1] = q[1] + 1; RX_MN - t[1] >= q[2] + q[3] + 2; ++t[1])
        for (t[2] = q[2] + 1; RX_MN - t[1] - t[2] >= q[3] + 1; ++t[2])
        {
            t[3] = RX_MN - t[1] - t[2];
            sizeOfW1();

            unsigned long long l_subBlockLength = combinationTable[RX_MN - t[1]][t[2]];
            unsigned long long blockTimes = RX_APPROXIMATE_BLOCK_LENGTH / l_subBlockLength + 1;
            unsigned long long l_blockLength = l_subBlockLength * blockTimes;
            unsigned long long blockNum = W1Size / l_blockLength + (W1Size % l_blockLength > 0 ? 1 : 0);
            
            sprintf(sz,
                    "INSERT INTO `task_info` (`CombinationNum`, `BlockNum`, `W1Size`, `BlockLength`) "
                    "VALUES(%llu, %llu, %llu, %llu);\n",
                    currentCombination, blockNum, W1Size, l_blockLength);
            ftaskinfo << sz;

            sprintf(sz, "\t{%llu, %llu, %llu},\n", blockNum, W1Size, l_blockLength);
            farray << sz;

            for (unsigned long long currentBlock = 0; currentBlock < blockNum; ++currentBlock)
            {
                if (layer3TaskNum++ % RX_LAYER1_BLOCKS_PER_TASK == 0)
                {
                    sprintf(sz,
                            "INSERT INTO `task` (`CombinationNum`, `Block`, `Layer`) VALUES(%llu, %llu, %d);\n",
                            currentCombination, currentBlock, 1);
                    ftasks << sz;
                    ++layer1TaskNum;
                }
                /*sprintf(sz,
                        "INSERT INTO `task` (`CombinationNum`, `Block`, `Layer`) VALUES(%llu, %llu, %d);\n",
                        currentCombination, currentBlock, 3);
                ftasks << sz;*/
            }
            ++currentCombination;
        }
    farray << "\t{}\n};";
    ftaskinfo << "/* layer1TaskNum: " << layer1TaskNum << " */\n";
    ftaskinfo << "/* layer3TaskNum: " << layer3TaskNum << " */";
}

bool operator==(const RXTASKINFO &lhs, const RXTASKINFO &rhs)
{
    static const std::size_t builtInPartSize =
            reinterpret_cast<const char *>(&lhs.resultBits) - reinterpret_cast<const char *>(&lhs);

    return !std::memcmp(&lhs, &rhs, builtInPartSize) && lhs.resultBits == rhs.resultBits;
}

std::ofstream &operator<<(std::ofstream &lhs, const RXTASKINFO &rhs)
{
    static const std::size_t builtInPartSize =
            reinterpret_cast<const char *>(&rhs.resultBits) - reinterpret_cast<const char *>(&rhs);
    //static const std::size_t bitsetSize = sizeof (rhs.resultBits);

    lhs.write(reinterpret_cast<const char *>(&rhs), builtInPartSize);
    if (!lhs)
        return lhs;

    const unsigned int cnt = rhs.resultBits.count();
    lhs.write(reinterpret_cast<const char *>(&cnt), sizeof (cnt));
    for (unsigned int i = 0; lhs && i < RX_LAYER1_BLOCKS_PER_TASK; ++i)
        if (rhs.resultBits[i])
            lhs.write(reinterpret_cast<const char *>(&i), sizeof (i));

    //lhs.write(reinterpret_cast<const char *>(&rhs), builtInPartSize + bitsetSize);

    return lhs;
}

std::ifstream &operator>>(std::ifstream &lhs, RXTASKINFO &rhs)
{
    static const std::size_t builtInPartSize =
            reinterpret_cast<const char *>(&rhs.resultBits) - reinterpret_cast<const char *>(&rhs);
    //static const std::size_t bitsetSize = sizeof (rhs.resultBits);

    if (builtInPartSize != lhs.read(reinterpret_cast<char *>(&rhs), builtInPartSize).gcount())
        return lhs;

    unsigned int cnt = 0;
    if (sizeof (cnt) != lhs.read(reinterpret_cast<char *>(&cnt), sizeof (cnt)).gcount())
        return lhs;
    unsigned temp = 0;
    while (cnt--)
    {
        if (sizeof (temp) != lhs.read(reinterpret_cast<char *>(&temp), sizeof (temp)).gcount())
            return lhs;
        rhs.resultBits.set(temp);
    }

    //lhs.read(reinterpret_cast<char *>(&rhs), builtInPartSize + bitsetSize);

    return lhs;
}
