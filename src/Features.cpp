/*  GRAPHITENG LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#include "graphiteng/Features.h"
#include "FeaturesImp.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

GRNG_EXPORT void DeleteFeatures(Features *p)
{
    delete p;
}


FeaturesHandle FeaturesHandle::clone() const		//clones the Features which are then owned separately
{
    if (ptr())
	return ptr()->clone();
    else
	return FeaturesHandle();
}


bool FeaturesHandle::maskedOr(const FeaturesHandle& other, const FeaturesHandle& mask) const	//returns false iff any of the FeaturesHandles are IsNull
{
    if (isNull())
	return false;
    if (other.isNull())
	return false;
    if (mask.isNull())
	return false;
    
    ptr()->maskedOr(*other.ptr(), *mask.ptr());
    return true;
}

}}}} // namespace
