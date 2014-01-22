#include "stdafx.h"
#include "Graph.h"

Graph::Graph() : complexity(0)
{
}

void Graph::connect(int u, int v)
{
    setOfVertices.insert(u);
    setOfVertices.insert(v);
    setOfNeighbourVertices[u].insert(v);
    setOfNeighbourVertices[v].insert(u);
}

bool Graph::isConnected(int u, int v) const
{
	return setOfNeighbourVertices[u].contain(v);
}

int Graph::size() const
{
	return setOfVertices.size();
}

void Graph::clear()
{   
	for (int i = 0; i < p; i++)
		setOfNeighbourVertices[i].clear();
	setOfVertices.clear();
}

bool Graph::size2CliqueExists()
{
        for (int i = 0; i < p; i++)
                if (!(setOfNeighbourVertices[i].isEmpty()))
                                return true;
        return false;
}


bool Graph::size3CliqueExists()
{
	for (int i = 0; i < p; i++)
		for (int j = i + 1; j < p; j++)
			if (setOfNeighbourVertices[i].contain(j) && !(setOfNeighbourVertices[i] & setOfNeighbourVertices[j]).isEmpty())
				return true;

	return false;
}

bool Graph::size4CliqueExists()
{
	Set R, P(setOfVertices), X;
	return sNceHelper(R, P, X, 4);
}

bool Graph::size5CliqueExists()
{
	Set R, P(setOfVertices), X;
	return sNceHelper(R, P, X, 5);
}

bool Graph::size6CliqueExists()
{
	Set R, P(setOfVertices), X;
	return sNceHelper(R, P, X, 6);
}

bool Graph::sNceHelper(Set &R, Set &P, Set &X, int N)
{
	if (R.size() >= N)
		return true;
	else if (P.isEmpty() && X.isEmpty())
        return false;
    
    //choose a pivot vertex u in P ⋃ X
    P.resetIterator();
    int u = P.next(), v;
    
    Set P_(P);
    if (u >= 0)
        P_ -= setOfNeighbourVertices[u];
    
    Set _P, _X;
    for (P_.resetIterator(); (v = P_.next()) >= 0; ) {
        R.insert(v);
        _P = P & setOfNeighbourVertices[v];
        _X = X & setOfNeighbourVertices[v];
        
		if (sNceHelper(R, _P, _X, N))
			return true;
        
        R.remove(v);
        P.remove(v);
        X.insert(v);
    }
	return false;
}

/*int Graph::sizeOfMaxClique()
{
    maxCliqueSize = 1;
	complexity = 0;
    Set R, P(setOfVertices), X;
    
    BronKerbosch2(R, P, X);
    
    return maxCliqueSize;
}*/

/*void Graph::BronKerbosch2(Set &R, Set &P, Set &X)
{
	complexity++;

	if (P.isEmpty() && X.isEmpty() && maxCliqueSize < R.size()) {
        maxCliqueSize = R.size();
		return;
	}
    
    //choose a pivot vertex u in P ⋃ X
    P.resetIterator();
    int u = P.next(), v;
    
    Set P_(P);
    if (u >= 0)
        P_ -= setOfNeighbourVertices[u];
    
    Set _P, _X;
    for (P_.resetIterator(); (v = P_.next()) >= 0; ) {
        R.insert(v);
        _P = P & setOfNeighbourVertices[v];
        _X = X & setOfNeighbourVertices[v];
        
        BronKerbosch2(R, _P, _X);
        
        R.remove(v);
        P.remove(v);
        X.insert(v);
    }
}*/
