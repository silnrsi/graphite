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
#include "graphiteng/FeatureRef.h"
#include "FeatureMap.h"

using namespace org::sil::graphite::v2;

namespace org { namespace sil { namespace graphite { namespace v2 {

GRNG_EXPORT FeatureRef* make_FeatureRef(byte bits, byte index, uint32 mask)
{                      //When finished with the FeatureRef, call destroy_FeatureRef    
  return new FeatureRef(bits, index, mask);
}


GRNG_EXPORT FeatureRef* clone_FeatureRef(const FeatureRef*pfeatureref)
{                      //When finished with the FeatureRef, call destroy_FeatureRef    
    if (pfeatureref)
    return new FeatureRef(*pfeatureref);
    else
    return NULL;
}


GRNG_EXPORT void apply_value_to_feature(uint16 val, const FeaturesHandle& pDest, FeatureRef* pRes)
{
    if (!pRes)
    return;
    if (pDest.isNull())
    return;
    
    pRes->applyValToFeature(val, pDest.ptr());
}


GRNG_EXPORT void mask_feature(const FeatureRef* pfeatureref, const FeaturesHandle& pDest)
{
    if (!pfeatureref)
    return;
    if (pDest.isNull())
    return;
    
    pfeatureref->maskFeature(pDest.ptr());
}


GRNG_EXPORT uint16 get_feature_value(const FeatureRef*pfeatureref, const FeaturesHandle& feats)    //returns 0 if either pointer is NULL
{
    if (!pfeatureref)
    return 0;
    if (feats.isNull())
    return 0;
    
    return pfeatureref->getFeatureVal(*feats.ptr());
}


GRNG_EXPORT void destroy_FeatureRef(FeatureRef *p)
{
    delete p;
}









}}}} // namespace
