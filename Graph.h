#ifndef RX_GRAPH_H
#define RX_GRAPH_H

#include "RamseyXHeader.h"
#include <bitset>

class Graph
{
public:
	unsigned long long complexity;
    std::bitset<RX_P> setOfVertices;
    std::bitset<RX_P> setOfNeighbourVertices[RX_P];
    
public:
    Graph();
    void connect(int u, int v);
    bool isConnected(int u, int v) const;
    std::size_t order() const;
    void clear();
	bool size2CliqueExists();
	bool size3CliqueExists();
	bool size4CliqueExists();
	bool size5CliqueExists();
	bool size6CliqueExists();
    bool sNceHelper(std::bitset<RX_P> &R, std::bitset<RX_P> &P, std::bitset<RX_P> &X, unsigned int N);
};

#endif
