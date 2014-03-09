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
#include "graph.h"
#include "bitsetiterator.h"

bool Graph::sNceHelper(std::bitset<RX_P> &R, std::bitset<RX_P> &P, std::bitset<RX_P> &X, unsigned int N)
{
    if (R.count() >= N)
        return true;
    else if (P.none() && X.none())
        return false;
    
    // Choose a pivot vertex u in P & X
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
