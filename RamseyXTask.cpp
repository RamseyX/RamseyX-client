#include "RamseyXTask.h"
#include "BitsetIterator.h"
#include <cstring>
#include <fstream>

unsigned long long taskInfo[][3] = {
	{1, 43108065, 50019606},
	{5, 215540325, 50114610},
	{18, 862161300, 50352120},
	{55, 2832815700, 51505740},
	{152, 7790243175, 51505740},
	{363, 18177234075, 50075025},
	{605, 36354468150, 60090030},
	{908, 62794081350, 69194580},
	{1815, 94191122025, 51895935},
	{1815, 123173005725, 67863915},
	{1815, 140769149400, 77558760},
	{1815, 140769149400, 77558760},
	{1815, 123173005725, 67863915},
	{1815, 94191122025, 51895935},
	{908, 62794081350, 69194580},
	{605, 36354468150, 60090030},
	{363, 18177234075, 50075025},
	{152, 7790243175, 51505740},
	{55, 2832815700, 51505740},
	{5, 221457600, 50020425},
	{22, 1062996480, 50024520},
	{82, 4074819840, 50106420},
	{252, 12806576640, 50913720},
	{637, 33617263680, 52837785},
	{1352, 74705030400, 55255200},
	{2704, 141939557760, 52492440},
	{3606, 232264730880, 64422540},
	{5408, 329041702080, 60843510},
	{5408, 404974402560, 74884320},
	{5408, 433901145600, 80233200},
	{5408, 404974402560, 74884320},
	{5408, 329041702080, 60843510},
	{3606, 232264730880, 64422540},
	{2704, 141939557760, 52492440},
	{1352, 74705030400, 55255200},
	{637, 33617263680, 52837785},
	{252, 12806576640, 50913720},
	{20, 957615750, 50017500},
	{89, 4405032450, 50052600},
	{323, 16151785650, 50025690},
	{958, 48455356950, 50617710},
	{2373, 121138392375, 51061725},
	{4961, 255736606125, 51555075},
	{9095, 460325891025, 50617710},
	{13642, 711412740675, 52151580},
	{18189, 948550320900, 52151580},
	{18189, 1094481139500, 60174900},
	{18189, 1094481139500, 60174900},
	{18189, 948550320900, 52151580},
	{13642, 711412740675, 52151580},
	{9095, 460325891025, 50617710},
	{4961, 255736606125, 51555075},
	{2373, 121138392375, 51061725},
	{958, 48455356950, 50617710},
	{71, 3527856150, 50007750},
	{311, 15522567060, 50058580},
	{1083, 54328984710, 50190140},
	{3065, 155225670600, 50650600},
	{7151, 368660967675, 51555075},
	{13881, 737321935350, 53117350},
	{23598, 1253447290095, 53117350},
	{33711, 1823196058320, 54083120},
	{39330, 2278995072900, 57946200},
	{47196, 2454302386200, 52003000},
	{39330, 2278995072900, 57946200},
	{33711, 1823196058320, 54083120},
	{23598, 1253447290095, 53117350},
	{13881, 737321935350, 53117350},
	{7151, 368660967675, 51555075},
	{3065, 155225670600, 50650600},
	{224, 11175806950, 50005450},
	{938, 46938389190, 50048460},
	{3122, 156461297300, 50119300},
	{8414, 424680664100, 50473500},
	{18798, 955531494225, 50834025},
	{35339, 1804892822425, 51074375},
	{55217, 2887828515880, 52300160},
	{73622, 3937947976200, 53488800},
	{88347, 4594272638900, 52003000},
	{88347, 4594272638900, 52003000},
	{73622, 3937947976200, 53488800},
	{55217, 2887828515880, 52300160},
	{35339, 1804892822425, 51074375},
	{18798, 955531494225, 50834025},
	{8414, 424680664100, 50473500},
	{613, 30615291168, 50005956},
	{2448, 122461164672, 50027208},
	{7746, 387793688128, 50069712},
	{19871, 997183769472, 50185080},
	{42371, 2119015510128, 50012028},
	{73877, 3767138684672, 50992656},
	{110815, 5650708027008, 50992656},
	{137199, 7191810216192, 52419024},
	{151641, 7791127734208, 51378964},
	{137199, 7191810216192, 52419024},
	{110815, 5650708027008, 50992656},
	{73877, 3767138684672, 50992656},
	{42371, 2119015510128, 50012028},
	{19871, 997183769472, 50185080},
	{1454, 72697619610, 50004185},
	{5525, 276250954518, 50002414},
	{16552, 828752863554, 50069712},
	{40245, 2012685525774, 50012028},
	{80489, 4025371051548, 50012028},
	{132416, 6708951752580, 50665780},
	{186586, 9392532453612, 50338904},
	{221886, 11100265626996, 50026886},
	{221886, 11100265626996, 50026886},
	{186586, 9392532453612, 50338904},
	{132416, 6708951752580, 50665780},
	{80489, 4025371051548, 50012028},
	{40245, 2012685525774, 50012028},
	{2993, 149621448900, 50005340},
	{10771, 538637216040, 50008266},
	{30483, 1526138778780, 50065323},
	{69572, 3488317208640, 50139936},
	{130281, 6540594766200, 50203890},
	{202516, 10174258525200, 50239420},
	{262232, 13226536082760, 50438388},
	{288086, 14428948453920, 50085672},
	{262232, 13226536082760, 50438388},
	{202516, 10174258525200, 50239420},
	{130281, 6540594766200, 50203890},
	{69572, 3488317208640, 50139936},
	{5331, 266553472185, 50004675},
	{18120, 906281805429, 50017842},
	{48305, 2416751481144, 50031408},
	{103575, 5178753173880, 50000400},
	{181045, 9062818054290, 50058540},
	{260450, 13090737189530, 50262030},
	{313641, 15708884627436, 50085672},
	{313641, 15708884627436, 50085672},
	{260450, 13090737189530, 50262030},
	{181045, 9062818054290, 50058540},
	{103575, 5178753173880, 50000400},
	{8213, 410603560965, 50000400},
	{26279, 1313931395088, 50000400},
	{65697, 3284828487720, 50000400},
	{131393, 6569656975440, 50000400},
	{213471, 10675692585090, 50010090},
	{284389, 14234256780120, 50052080},
	{312723, 15657682458132, 50068876},
	{284389, 14234256780120, 50052080},
	{213471, 10675692585090, 50010090},
	{131393, 6569656975440, 50000400},
	{10942, 547055108964, 50000400},
	{32824, 1641165326892, 50000400},
	{76582, 3829385762748, 50004276},
	{142135, 7111716416532, 50035284},
	{213202, 10667574624798, 50035284},
	{260405, 13038146763642, 50068876},
	{260405, 13038146763642, 50068876},
	{213202, 10667574624798, 50035284},
	{142135, 7111716416532, 50035284},
	{12623, 631141692300, 50000400},
	{35342, 1767196738440, 50002848},
	{76562, 3828926266620, 50011416},
	{131206, 6563873599920, 50027328},
	{180451, 9025326199890, 50015394},
	{200443, 10028140222100, 50029980},
	{180451, 9025326199890, 50015394},
	{131206, 6563873599920, 50027328},
	{12623, 631134695100, 50001420},
	{32816, 1640950207260, 50005228},
	{65624, 3281900414520, 50011416},
	{103144, 5157272079960, 50000808},
	{128918, 6446590099950, 50005670},
	{128918, 6446590099950, 50005670},
	{103144, 5157272079960, 50000808},
	{10940, 546983156720, 50000860},
	{26255, 1312759576128, 50000496},
	{48133, 2406725889568, 50001952},
	{68758, 3438179842240, 50004240},
	{77340, 3867952322520, 50012820},
	{68758, 3438179842240, 50004240},
	{8205, 410237366175, 50001315},
	{18050, 902522205585, 50002953},
	{30082, 1504203675975, 50004955},
	{38675, 1933976154825, 50006385},
	{38675, 1933976154825, 50006385},
	{5309, 265447707525, 50000951},
	{10618, 530895415050, 50001952},
	{15926, 796343122575, 50002953},
	{18202, 910106425800, 50000808},
	{2950, 147470948625, 50000665},
	{5309, 265447707525, 50001237},
	{7079, 353930276700, 50000808},
	{1398, 69854659875, 50000445},
	{2236, 111767455800, 50000544},
	{559, 27941863950, 50000280}
};

bool RamseyXTask::firstInstance = true;
int RamseyXTask::Zp[RX_P] = {};
unsigned int RamseyXTask::absOfPrimitiveRootPowerTable[RX_MN + 1] = {};
unsigned int RamseyXTask::absTable[RX_P] = {};
unsigned int RamseyXTask::q[RX_N + 1] = {};
unsigned long long RamseyXTask::combinationTable[RX_MN][RX_MN] = {};

void RamseyXTask::init()
{
	if (firstInstance)
	{
		firstInstance = false;
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
}

RamseyXTask::RamseyXTask() :
	found(false),
	W1Size(0),
    blockLength(0),
    subBlockLength(0),
	blockBegin(0),
	blockEnd(0),
    absOffset(0)
{
    gen1[0] = 0;

	init();
}

int RamseyXTask::abs(unsigned int x)
{
    x %= RX_P;
    return RX_P < (x << 1) ? RX_P - x : x;
}

void RamseyXTask::t_launch(RXTASKINFO &info, RXFLAG &threadFlag)
{
	// Layer 1
	if (info.layer == 1)
	{
		// When a new task is added, info.combinationNum and info.block indicate
		// the beginning block (inclusive). Afterwards, they indicate the block
		// begin currently processed.
		// A layer-1 task will span LAYER1_BLOCKS_PER_TASK blocks unless the number of
		// remaining blocks is less than LAYER1_BLOCKS_PER_TASK.
		
		unsigned long long currentComb = 0;
        for (t[1] = q[1] + 1; RX_MN - t[1] >= q[2] + q[3] + 2; ++t[1])
            for (t[2] = q[2] + 1; RX_MN - t[1] - t[2] >= q[3] + 1;
				++t[2], ++currentComb, ++info.combinationNum)
			{
                if (info.offset >= RX_LAYER1_BLOCKS_PER_TASK)
				{
                    info.result = RX_LAUNCH_SUCCESS;
					return;
				}
				if (currentComb < info.combinationNum)
				{
					--info.combinationNum;
					continue;
				}
				// currentComb == info.combinationNum
                t[3] = RX_MN - t[1] - t[2];
				if (t[3] < q[3] + 1)
				{
                    info.result = RX_LAUNCH_SUCCESS;
					return;
				}

                subBlockLength = combinationTable[RX_MN - t[1]][t[2]];
                blockLength = taskInfo[info.combinationNum][RX_TASKINFO_BLOCKLENGTH];
                W1Size = taskInfo[info.combinationNum][RX_TASKINFO_W1SIZE];

                for (info.block = 0; info.block < taskInfo[info.combinationNum][RX_TASKINFO_BLOCKNUM] &&
                    info.offset < RX_LAYER1_BLOCKS_PER_TASK; ++info.block, ++info.offset)
				{
					// [blockBegin, blockEnd)
					blockBegin = info.block * blockLength;
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
					absOffset = 0;

					// generateB1(1)
                    for (gen1[1] = 1; gen1[1] * (t[1] - 1) < RX_MN; ++gen1[1])
						if (!t_generateB1Only(2, info, threadFlag))
							break;

					if (found)
						info.resultBits.set(static_cast<std::size_t>(info.offset));
					else
						info.resultBits.reset(static_cast<std::size_t>(info.offset));
				}
			}
        info.result = RX_LAUNCH_SUCCESS;
		return;
	}

	// Layer 3
	unsigned long long i = 0;
    for (t[1] = q[1] + 1; RX_MN - t[1] >= q[2] + q[3] + 2; ++t[1])
        for (t[2] = q[2] + 1; RX_MN - t[1] - t[2] >= q[3] + 1; ++t[2], ++i)
			if (i == info.combinationNum)
				goto COMB_FOUND;
COMB_FOUND:
    t[3] = RX_MN - t[1] - t[2];
	if (t[3] < q[3] + 1)
	{
        info.result = RX_LAUNCH_INVALID_ARGUMENTS;
		return;
	}

    subBlockLength = combinationTable[RX_MN - t[1]][t[2]];
    blockLength = taskInfo[info.combinationNum][RX_TASKINFO_BLOCKLENGTH];
    W1Size = taskInfo[info.combinationNum][RX_TASKINFO_W1SIZE];
	blockBegin = info.block * blockLength;
	if (W1Size < blockBegin + blockLength)
	{
		blockEnd = W1Size;
		blockLength = blockEnd - blockBegin;
	}
	else
		blockEnd = blockBegin + blockLength;
	if (blockBegin >= blockEnd)
	{
        info.result = RX_LAUNCH_INVALID_ARGUMENTS;
		return;
	}

	found = false;
	absOffset = 0;

	// generateB1(1)
    for (gen1[1] = 1; gen1[1] * (t[1] - 1) < RX_MN; ++gen1[1])
		if (!t_generateB1(2, info, threadFlag))
			break;

	if (found)
        info.result = RX_LAUNCH_SUCCESS;
	else if (absOffset >= blockEnd)
        info.result = RX_LAUNCH_REACHED_BLOCK_END;
}

bool RamseyXTask::t_generateB1(unsigned int index, RXTASKINFO &info, RXFLAG &threadFlag)
{
	if (index == t[1])
	{
		if (/*!(info.complexity & 15) && */*(threadFlag.termFlag))
			return false;
		
		// S1 done; S2 to be generated
		if (absOffset + subBlockLength <= blockBegin + info.offset)
		{
			absOffset += subBlockLength;
			return true;
		}

		++info.complexity;

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
		if (GpA[1].size2CliqueExists())
		{
			absOffset += subBlockLength;
			info.offset += subBlockLength;
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
			if (!t_generateB2(1, info))
				return false;

		absOffset += subBlockLength;
		info.offset += subBlockLength;
		return absOffset < blockEnd;
	}
	for (gen1[index] = gen1[index - 1] + gen1[1];
        gen1[index] + gen1[1] * (t[1] - 1 - index) < RX_MN;
		++gen1[index])
		if (!t_generateB1(index + 1, info, threadFlag))
			return false;
	
	return true;
}

bool RamseyXTask::t_generateB1Only(unsigned int index, RXTASKINFO &info, RXFLAG &threadFlag)
{
	if (index == t[1])
	{
		if (!(info.complexity & 15) && *(threadFlag.termFlag))
			return false;
		
		// S1 done
		++info.complexity;

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
		if (GpA[1].size2CliqueExists())
		{
			absOffset += subBlockLength;
			return absOffset < blockEnd;
		}
		
		// Qualified S[1] found
		found = true;
		return false;
	}
	for (gen1[index] = gen1[index - 1] + gen1[1];
        gen1[index] + gen1[1] * (t[1] - 1 - index) < RX_MN;
		++gen1[index])
		if (!t_generateB1Only(index + 1, info, threadFlag))
			return false;

	return true;
}

bool RamseyXTask::t_generateB2(unsigned int index, RXTASKINFO &info)
{
	if (index == t[2])
	{
		++info.complexity;

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
		if (GpA[2].size2CliqueExists())
			return true;

		// S2 done; S3 to be generated
		std::memset(inB3, true, sizeof (inB3));
		for (i = 0; i < t[1]; ++i)
			inB3[gen1[i]] = false;
		for (i = 0; i < t[2]; ++i)
			inB3[gen2Source[gen2Indices[i]]] = false;

		return t_generateB3(info);
	}
	for (gen2Indices[index] = gen2Indices[index - 1] + 1;
		gen2Indices[index] < t[2] + t[3];
		++gen2Indices[index])
		if (!t_generateB2(index + 1, info))
			return false;

	return true;
}

bool RamseyXTask::t_generateB3(RXTASKINFO &info)
{
	++info.complexity;

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
	if (GpA[3].size5CliqueExists())
		return true;

	// found!
	found = true;
	return false;
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

void RamseyXTask::constructSiFromBi(unsigned int i)
{
	S[i].reset();
    for (unsigned int j = 0; j < RX_MN; ++j)
        if (B[i][j])
			S[i].set(absOfPrimitiveRootPowerTable[j]);
}

void RamseyXTask::initZp()
{
    for (unsigned int i = 0; i < RX_P; ++i)
        Zp[i] = static_cast<int>(i) - static_cast<int>(RX_MN);
}

int RamseyXTask::indexOfZp(int element)
{
    return element + RX_MN;
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

unsigned long long RamseyXTask::combination(unsigned long long i, unsigned long long j)
{
    if (j == 1)
        return i;
    if (i == j)
        return 1;
    return combination(i - 1, j - 1) + combination(i - 1, j);
}

void RamseyXTask::printSQLScript()
{
	std::ofstream fout("C:\\Users\\Tai\\Desktop\\task_info.txt");
	std::ofstream fout2("C:\\Users\\Tai\\Desktop\\task.txt");
	std::ofstream fout3("C:\\Users\\Tai\\Desktop\\array.txt");
	fout3 << "unsigned long long taskInfo[][3] = {\n";
	char sz[200] = {};
    unsigned long long counter = 0;
    for (t[1] = q[1] + 1; RX_MN - t[1] >= q[2] + q[3] + 2; ++t[1])
        for (t[2] = q[2] + 1; RX_MN - t[1] - t[2] >= q[3] + 1; ++t[2])
		{
            t[3] = RX_MN - t[1] - t[2];
            sizeOfW1();

            unsigned long long l_subBlockLength = combinationTable[RX_MN - t[1]][t[2]];
            unsigned long long blockTimes = RX_APPROXIMATE_BLOCK_LENGTH / l_subBlockLength + 1;
			unsigned long long l_blockLength = l_subBlockLength * blockTimes;
			unsigned long long blockNum = W1Size / l_blockLength + (W1Size % l_blockLength > 0 ? 1 : 0);
			
			sprintf_s(sz, 200,
				"INSERT INTO `taskinfo` (`CombinationNum`, `BlockNum`, `W1Size`, `BlockLength`) VALUES(%llu, %llu, %llu, %llu);\n",
				counter, blockNum, W1Size, l_blockLength);
			fout << sz;
			sprintf_s(sz, 200,
				"\t{%llu, %llu, %llu},\n", blockNum, W1Size, l_blockLength);
			fout3 << sz;
			/*for (unsigned long long i = 0; i < blockNum; ++i)
			{
				sprintf_s(sz, 200, "INSERT INTO `task` (`CombinationNum`, `Block`) VALUES(%llu, %llu);\n", counter, i);
				fout2 << sz;
			}*/
			++counter;
		}
	fout3 << "\t{}\n};";
}
