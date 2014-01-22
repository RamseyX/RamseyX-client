#include "stdafx.h"
//#include <iostream>
#include <fstream>
#include "RamseyXTask.h"

int RXBATCHINFO::counter = 1;

RXBATCHINFO::RXBATCHINFO() :
	identifier(0),
	numOfTasks(0),
	clockID(0),
	deadline(0)
{
	memset(taskID, 0, sizeof (taskID));
	memset(taskCompleted, false, sizeof (taskCompleted));
}

bool RamseyXTask::firstInstance = true;
int RamseyXTask::Zp[p] = {0};
int RamseyXTask::absOfPrimitiveRootPowerTable[mn + 1] = {0};
int RamseyXTask::absTable[p] = {0};
int RamseyXTask::q[n + 1] = {0};
unsigned __int64 RamseyXTask::combinationTable[mn][mn] = {0};

void RamseyXTask::init()
{
	if (firstInstance) {
		firstInstance = false;
		q[1] = q1;
		q[2] = q2;
		q[3] = q3;
		initZp();
		for (int i = 0; i <= mn; i++)
			absOfPrimitiveRootPowerTable[i] = absOfPrimitiveRootPower(i);
		for (int i = 0; i < p; i++)
			absTable[i] = abs(i);
		for (int i = 1; i < mn; i++) {
			combinationTable[i][1] = i;
			combinationTable[i][i] = combinationTable[i][0] = 1;
		}
		for (int i = 1; i < mn; i++)
			for (int j = 1; j < i; j++)
				combinationTable[i][j] = combinationTable[i - 1][j - 1] + combinationTable[i - 1][j];
	}
}

RamseyXTask::RamseyXTask() : 
	located(false), reachedBlockEnd(false), found(false), 
	W1Size(0), progress(0), blockBegin(0), blockEnd(0),
	blockLength(0), subBlockLength(0), complexity(0)
{
    gen1[0] = 0;

	init();
}

int RamseyXTask::abs(int x)
{
    x %= p;
    return p < (x << 1) ? p - x : x;
}

void RamseyXTask::launch(RXTASKINFO &info) // block >= 0, combNum >= 0
{
	located = false;

    unsigned __int64 i = 0;
    for (t[1] = q[1] + 1; mn - t[1] >= q[2] + q[3] + 2; t[1]++)
        for (t[2] = q[2] + 1; mn - t[1] - t[2] >= q[3] + 1; t[2]++, i++)
            if (i >= info.combinationNum)
                goto DONE;
DONE:
    t[3] = mn - t[1] - t[2];
    if (t[3] < q[3] + 1) {
        info.result = LAUNCH_INVALID_ARGUMENTS;
		return;
	}
    
    subBlockLength = combinationTable[mn - t[1]][t[2]];
	unsigned __int64 blockTimes = APPROXIMATE_BLOCK_LENGTH / subBlockLength + 1;
	blockLength = subBlockLength * blockTimes;
    sizeOfW1();
    blockBegin = info.block * blockLength;
	if (W1Size + 1 < blockBegin + blockLength) {
        blockEnd = W1Size;
        blockLength = blockEnd - blockBegin;
    }
    else
        blockEnd = blockBegin + blockLength;
    if (blockBegin >= blockEnd) {
        info.result = LAUNCH_INVALID_ARGUMENTS;
		return;
	}
    
    reachedBlockEnd = false;
    found = false;
    progress = 0;
	offset = info.offset;
	complexity = 0;
		
    // generateB1(1)
    for (gen1[1] = 1; !reachedBlockEnd && gen1[1] * (t[1] - 1) < mn; gen1[1]++)
        generateB1(2);
    
	located = false;

    if (found) {
		//info.complexity += complexity;
        info.result = LAUNCH_SUCCESS;
	}
    else {
		//info.complexity += complexity;
        info.result = LAUNCH_REACHED_BLOCK_END;
	}
}

void RamseyXTask::generateB1(int index)
{
    if (index == t[1]) {
        // S1 done; S2 to be generated
        if (progress + subBlockLength <= blockBegin + offset) {
            progress += subBlockLength;
            return;
        }
		
		located = true;
		complexity++;

		int i, j, k;
		// validate B[1] (GpS[1])
		B[1].clear();
		for (i = 0; i < t[1]; i++)
			B[1].insert(gen1[i]);
		constructSiFromBi(1);
		constructAiFromSi(1);

		GpA[1].clear();
		for (j = 0; j < p; j++)
			if (A[1].contain(j))
				for (k = j + 1; k < p; k++)
					if (A[1].contain(k) && S[1].contain(absTable[Zp[k] - Zp[j]]))
						GpA[1].connect(j, k);
		if (GpA[1].size2CliqueExists()) {
			progress += subBlockLength;
			if (progress >= blockEnd)
				reachedBlockEnd = true;
			return;
		}
        
		// prepare for B[2]
        memset(inB1, false, sizeof (inB1));
        for (i = 0; i < t[1]; i++)
            inB1[gen1[i]] = true;
        for (i = 1, j = 0; i < mn; i++)
            if (!inB1[i])
                gen2Source[j++] = i;
        
        // generateB2(1)
        for (gen2Indices[0] = 0; !reachedBlockEnd && gen2Indices[0] < t[2] + t[3]; gen2Indices[0]++)
            generateB2(1);
        
        progress += subBlockLength;
        if (progress >= blockEnd)
            reachedBlockEnd = true;
        return;
    }
    for (gen1[index] = gen1[index - 1] + gen1[1]; !reachedBlockEnd && gen1[index] + gen1[1] * (t[1] - 1 - index) < mn; gen1[index]++)
        generateB1(index + 1);
}

void RamseyXTask::generateB2(int index)
{
    if (index == t[2]) {
		complexity++;

		//validate B[2] (GpS[2])
		int i, j, k;
		B[2].clear();
		for (i = 0; i < t[2]; i++)
			B[2].insert(gen2Source[gen2Indices[i]]);
		constructSiFromBi(2);
		constructAiFromSi(2);

		GpA[2].clear();
		for (j = 0; j < p; j++)
			if (A[2].contain(j))
				for (k = j + 1; k < p; k++)
					if (A[2].contain(k) && S[2].contain(absTable[Zp[k] - Zp[j]]))
						GpA[2].connect(j, k);
		if (GpA[2].size2CliqueExists())
			return;

        // S2 done; S3 to be generated
        memset(inB3, true, sizeof (inB3));
        for (i = 0; i < t[1]; i++)
            inB3[gen1[i]] = false;
        for (i = 0; i < t[2]; i++)
            inB3[gen2Source[gen2Indices[i]]] = false;
        
        generateB3();
        
        return;
    }
    for (gen2Indices[index] = gen2Indices[index - 1] + 1; !reachedBlockEnd && gen2Indices[index] < t[2] + t[3]; gen2Indices[index]++)
        generateB2(index + 1);
}

void RamseyXTask::generateB3()
{
	complexity++;
    int i, j, k;
    for (i = 1, j = 0; i < mn; i++)
        if (inB3[i])
            gen3[j++] = i;

	// validate B[3]
	B[3].clear();
    for (i = 0; i < t[3]; i++)
        B[3].insert(gen3[i]);
	constructSiFromBi(3);
	constructAiFromSi(3);

	GpA[3].clear();
	for (j = 0; j < p; j++)
		if (A[3].contain(j))
			for (k = j + 1; k < p; k++)
				if (A[3].contain(k) && S[3].contain(absTable[Zp[k] - Zp[j]]))
					GpA[3].connect(j, k);
	if (GpA[3].size5CliqueExists())
		return;

	// found!
	found = true;
	reachedBlockEnd = true;
	/*std::ofstream fout("c:\\users\\tai\\desktop\\log.txt");
	for (int i = 1; i <= n; i++)
	{
		fout << 'B' << i << ": { ";
		for (int j = 0; j <= p; j++)
			if (B[i].contain(j))
				fout << j << ' ';
		fout << "}\n";
	}
	fout.close();*/
}

void RamseyXTask::calcSizeOfW1(int index)
{
    if (index == t[1] - 1) {
        W1Size += mn -  a[index - 1] - a[1];
        return;
    }
    for (a[index] = a[index - 1] + a[1]; a[index] + a[1] * (t[1] - 1 - index) < mn; a[index]++)
        calcSizeOfW1(index + 1);
}

unsigned __int64 RamseyXTask::sizeOfW1()
{
    W1Size = 0;
    for (a[1] = 1; a[1] * (t[1] - 1) < mn; a[1]++)
        calcSizeOfW1(2);
    W1Size *= combinationTable[mn - t[1]][t[2]];
    return W1Size;
}

int RamseyXTask::absOfPrimitiveRootPower(int exp)
{
    // |g^exp|
    int result = 1;
    for (int i = 0; i < exp; i++) {
        result *= g;
        result %= p;
    }
    
    return p < (result << 1) ? p - result : result;
}

void RamseyXTask::constructSiFromBi(int i)
{
	S[i].clear();
	for (int j = 0; j < mn; j++)
		if (B[i].contain(j))
			S[i].insert(absOfPrimitiveRootPowerTable[j]);
}

void RamseyXTask::initZp()
{
    for (int i = 0; i < p; i++)
        Zp[i] = i - mn;
}

int RamseyXTask::indexOfZp(int element)
{
    return element + mn;
}

void RamseyXTask::constructAiFromSi(int i)
{
	int j;
	A[i].clear();
	for (S[i].resetIterator(); (j = S[i].next()) >= 0; ) {
		A[i].insert(indexOfZp(j));
		A[i].insert(indexOfZp(-j));
	}
}

unsigned __int64 RamseyXTask::combination(unsigned __int64 i, unsigned __int64 j)
{
    if (j == 1)
        return i;
    if (i == j)
        return 1;
    return combination(i - 1, j - 1) + combination(i - 1, j);
}

void RamseyXTask::printSQLScript()
{
	char sz[200];
	sprintf_s(sz, 200, "c:\\users\\tai\\desktop\\task_info.txt");
	std::ofstream fout(sz);
	sprintf_s(sz, 200, "c:\\users\\tai\\desktop\\task.txt");
	std::ofstream fout2(sz);
    unsigned __int64 counter = 0;
    for (t[1] = q[1] + 1; mn - t[1] >= q[2] + q[3] + 2; t[1]++)
        for (t[2] = q[2] + 1; mn - t[1] - t[2] >= q[3] + 1; t[2]++) {
            t[3] = mn - t[1] - t[2];
            sizeOfW1();

			unsigned __int64 l_subBlockLength = combination(mn - t[1], t[2]);
			unsigned __int64 blockTimes = APPROXIMATE_BLOCK_LENGTH / l_subBlockLength + 1;
			unsigned __int64 l_blockLength = l_subBlockLength * blockTimes;
			unsigned __int64 blockNum = W1Size / l_blockLength + (W1Size % l_blockLength > 0 ? 1 : 0);
			
			sprintf_s(sz, 200, "INSERT INTO `taskinfo` (`CombinationNum`, `BlockNum`, `W1Size`, `BlockLength`) VALUES(%llu, %llu, %llu, %llu);\n", counter, blockNum, W1Size, l_blockLength);
			fout << sz;
			for (unsigned __int64 i = 0; i < blockNum; i++) {
				sprintf_s(sz, 200, "INSERT INTO `task` (`CombinationNum`, `Block`) VALUES(%llu, %llu);\n", counter, i);
				fout2 << sz;
			}
			counter++;
		}
	fout.close();
	fout2.close();
}
