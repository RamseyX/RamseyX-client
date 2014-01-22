#include "Graph.h"

#define LAUNCH_SUCCESS 0
#define LAUNCH_REACHED_BLOCK_END 1
#define LAUNCH_INVALID_ARGUMENTS 2

struct RXTASKINFO
{
	unsigned __int64 id;
	unsigned __int64 combinationNum;
	unsigned __int64 block;
	unsigned __int64 offset;
	unsigned __int64 blockLength;
	unsigned __int64 complexity;
	int result;
	int threadID;
	time_t deadline;
	int batch;
};

struct RXBATCHINFO
{
	int identifier;
	int numOfTasks;
	clock_t clockID;
	time_t deadline;
	int localCounter;
	unsigned __int64 taskID[TASKS_PER_BATCH];
	bool taskCompleted[TASKS_PER_BATCH];

	static int counter;

	RXBATCHINFO();
};

class RamseyXTask
{
public:
	static bool firstInstance;

    static int Zp[p]; // to be initialized
	static int absOfPrimitiveRootPowerTable[mn + 1]; // to be initialized
	static int absTable[p]; // to be initialized
    static int q[n + 1];
	static unsigned __int64 combinationTable[mn][mn];

    int t[n + 1]; // t1 + t2 + t3 = mn = 23
    int c[n + 1]; // ci = |Ai|
    Set S[n + 1]; // S[1 ~ 3] values: 1 ~ mn
    Set B[n + 1]; // B[1 ~ 3] values: 0 ~ mn - 1
    Set A[n + 1]; // A[1 ~ 3] values: 0 ~ p - 1
    Graph GpA[n + 1]; // Gp[Ai]
	Graph GpS[n + 1]; // Gp(Si)
    
    int a[p];
    int gen1[p]; // [0 ~ t[1] - 1]
    int gen2Indices[p];
    int gen2Source[p]; // [0 ~ t[2] - 1]
    int gen3[p]; // [0 ~ t[3] - 1]
    bool inB1[mn];
    bool inB3[mn];
    
    bool reachedBlockEnd;
    bool found;
	bool located;
    unsigned __int64 W1Size;
    unsigned __int64 blockLength;
    unsigned __int64 subBlockLength;
    unsigned __int64 progress;
    unsigned __int64 blockBegin;
    unsigned __int64 blockEnd;
	unsigned __int64 offset;
	unsigned __int64 complexity;
    
public:
    RamseyXTask();
    
	static void init();
	static void initZp();
	static int absOfPrimitiveRootPower(int exp);
    static int abs(int x);
    static unsigned __int64 combination(unsigned __int64 i, unsigned __int64 j);

    void launch(RXTASKINFO &info);
	void constructSiFromBi(int i);
	void constructAiFromSi(int i);

    void generateB1(int index);
    void generateB2(int index);
    void generateB3();
    
    unsigned __int64 sizeOfW1();
    void calcSizeOfW1(int index);
    int indexOfZp(int element);
    
    void printSQLScript();
};
