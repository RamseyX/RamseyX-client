#include "Graph.h"
#include "BitsetIterator.h"

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
