#include "Set.h"

class Graph
{
public:;
	unsigned __int64 complexity;
    Set setOfVertices;
    Set setOfNeighbourVertices[p];
    
public:
    Graph();
    void connect(int u, int v);
    bool isConnected(int u, int v) const;
    int size() const;
    void clear();
    //int sizeOfMaxClique();
    //void BronKerbosch2(Set &R, Set &P, Set &X);
	bool size2CliqueExists();
	bool size3CliqueExists();
	bool size4CliqueExists();
	bool size5CliqueExists();
	bool size6CliqueExists();
	bool sNceHelper(Set &R, Set &P, Set &X, int N);
};