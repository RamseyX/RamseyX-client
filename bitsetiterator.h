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
#ifndef RXBITSETITERATOR_H
#define RXBITSETITERATOR_H

#include <bitset>

template<std::size_t T>
class BitsetIterator
{
private:
    int iterator;
    const std::bitset<T> *bitvec;

public:
    BitsetIterator(const std::bitset<T> &bits) : iterator(-1), bitvec(&bits)
    {
    }
    
    void rebind(const std::bitset<T> &bits)
    {
        bitvec = &bits;
        reset();
    }
    
    void reset()
    {
        iterator = -1;
    }
    
    int next()
    {
        for (++iterator; static_cast<std::size_t>(iterator) < T && !bitvec->operator[](iterator); ++iterator);
        
        return static_cast<std::size_t>(iterator) < T ? iterator : -1;
    }
};

#endif // RXBITSETITERATOR_H
