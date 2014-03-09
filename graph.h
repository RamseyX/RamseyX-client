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
#ifndef RX_GRAPH_H
#define RX_GRAPH_H

#include "ramseyxdefs.h"
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
