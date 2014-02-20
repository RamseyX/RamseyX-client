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
    Graph() : complexity(0) { }
    void connect(int u, int v)
    {
        setOfVertices.set(u);
        setOfVertices.set(v);
        setOfNeighbourVertices[u].set(v);
        setOfNeighbourVertices[v].set(u);
    }
    bool isConnected(int u, int v) const
    {
        return setOfNeighbourVertices[u][v];
    }
    std::size_t order() const
    {
        return setOfVertices.count();
    }
    void clear()
    {
        for (unsigned int i = 0; i < RX_P; ++i)
            setOfNeighbourVertices[i].reset();
        setOfVertices.reset();
    }

    template<std::size_t cliqueOrder>
    bool cliqueExists()
    {
        std::bitset<RX_P> R, P(setOfVertices), X;
        return sNceHelper(R, P, X, cliqueOrder);
    }

private:
    bool sNceHelper(std::bitset<RX_P> &R, std::bitset<RX_P> &P, std::bitset<RX_P> &X, unsigned int N);
};

template<>
inline bool Graph::cliqueExists<2>()
{
    for (unsigned int i = 0; i < RX_P; ++i)
        if (setOfNeighbourVertices[i].any())
            return true;

    return false;
}

template<>
inline bool Graph::cliqueExists<3>()
{
    for (unsigned int i = 0; i < RX_P; i++)
        for (unsigned int j = i + 1; j < RX_P; j++)
            if (setOfNeighbourVertices[i][j] &&
                (setOfNeighbourVertices[i] & setOfNeighbourVertices[j]).any())
                return true;

    return false;
}

#endif
