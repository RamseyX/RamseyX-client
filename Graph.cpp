#include "Graph.h"
#include "BitsetIterator.h"

Graph::Graph() : complexity(0)
{
}

void Graph::connect(int u, int v)
{
    setOfVertices.set(u);
    setOfVertices.set(v);
    setOfNeighbourVertices[u].set(v);
    setOfNeighbourVertices[v].set(u);
}

bool Graph::isConnected(int u, int v) const
{
    return setOfNeighbourVertices[u][v];
}

std::size_t Graph::order() const
{
	return setOfVertices.count();
}

void Graph::clear()
{   
    for (unsigned int i = 0; i < RX_P; i++)
		setOfNeighbourVertices[i].reset();
	setOfVertices.reset();
}

bool Graph::size2CliqueExists()
{
    for (unsigned int i = 0; i < RX_P; i++)
		if (setOfNeighbourVertices[i].any())
			return true;

	return false;
}

bool Graph::size3CliqueExists()
{
    for (unsigned int i = 0; i < RX_P; i++)
        for (unsigned int j = i + 1; j < RX_P; j++)
            if (setOfNeighbourVertices[i][j] &&
				(setOfNeighbourVertices[i] & setOfNeighbourVertices[j]).any())
				return true;

	return false;
}

bool Graph::size4CliqueExists()
{
    std::bitset<RX_P> R, P(setOfVertices), X;
	return sNceHelper(R, P, X, 4U);
}

bool Graph::size5CliqueExists()
{
    std::bitset<RX_P> R, P(setOfVertices), X;
	return sNceHelper(R, P, X, 5U);
}

bool Graph::size6CliqueExists()
{
    std::bitset<RX_P> R, P(setOfVertices), X;
	return sNceHelper(R, P, X, 6U);
}

bool Graph::sNceHelper(std::bitset<RX_P> &R, std::bitset<RX_P> &P, std::bitset<RX_P> &X, unsigned int N)
{
	if (R.count() >= N)
		return true;
	else if (P.none() && X.none())
        return false;
    
    // Choose a pivot vertex u in P ⋃ X
    BitsetIterator<RX_P> iterator(P);
    int u = iterator.next(), v;
    
    std::bitset<RX_P> P_(P);
    if (u >= 0)
		P_ &= ~setOfNeighbourVertices[u];
    
    std::bitset<RX_P> _P, _X;
    for (iterator.rebind(P_); (v = iterator.next()) >= 0; )
	{
        R.set(v);
        _P = P & setOfNeighbourVertices[v];
        _X = X & setOfNeighbourVertices[v];
        
		if (sNceHelper(R, _P, _X, N))
			return true;
        
        R.reset(v);
        P.reset(v);
        X.set(v);
    }
	return false;
}
