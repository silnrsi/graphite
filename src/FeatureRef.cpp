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

GRNG_EXPORT uint32 fref_name(const FeatureRef* pfeatureref)    //returns 0 if pointer is NULL
{
  if (!pfeatureref)
    return 0;
  
  return pfeatureref->name();
}


GRNG_EXPORT uint16 fref_max_value(const FeatureRef* pfeatureref)    //returns 0 if pointer is NULL
{
  if (!pfeatureref)
    return 0;
  
  return pfeatureref->maxVal();
}


  
  
  
GRNG_EXPORT bool fref_set_feature_value(const FeatureRef* pfeatureref, uint16 val, Features* pDest)
{
    if (!pfeatureref)
    return false;
    if (!pDest)
    return false;
    
    return pfeatureref->applyValToFeature(val, pDest);
}


GRNG_EXPORT uint16 fref_feature_value(const FeatureRef*pfeatureref, const Features* feats)    //returns 0 if either pointer is NULL
{
    if (!pfeatureref)
    return 0;
    if (!feats)
    return 0;
    
    return pfeatureref->getFeatureVal(*feats);
}











}}}} // namespace
