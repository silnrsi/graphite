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
#include "graphiteng/FeatureRefHandle.h"
#include "FeatureMap.h"

using namespace org::sil::graphite::v2;

namespace org { namespace sil { namespace graphite { namespace v2 {

GRNG_EXPORT void DeleteFeatureRef(FeatureRef *p)
{
    delete p;
}

}}}} // namespace

FeatureRefHandle::FeatureRefHandle(byte bits, byte index, uint32 mask/*=0*/)
:	AutoHandle<FeatureRef, &DeleteFeatureRef>(new FeatureRef(bits, index, mask))
{
}


FeatureRefHandle FeatureRefHandle::clone() const		//clones the FeatureRef which are then owned separately
{
    if (ptr())
	return new FeatureRef(*ptr());
    else
	return NULL;
}


void FeatureRefHandle::applyValToFeature(uint16 val, const FeaturesHandle& pDest) const
{
    if (isNull())
	return;
    if (pDest.isNull())
	return;
    
    ptr()->applyValToFeature(val, pDest.ptr());
}


void FeatureRefHandle::maskFeature(const FeaturesHandle& pDest) const
{
    if (isNull())
	return;
    if (pDest.isNull())
	return;
    
    ptr()->maskFeature(pDest.ptr());
}


uint16 FeatureRefHandle::getFeatureVal(const FeaturesHandle& feats) const	//returns 0 if either handle IsNull
{
    if (isNull())
	return 0;
    if (feats.isNull())
	return 0;
    
    return ptr()->getFeatureVal(*feats.ptr());
}

