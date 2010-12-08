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
#include "graphite2/Font.h"
#include "FeaturesImp.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

GRNG_EXPORT GrFeatureVal* featureval_clone(const GrFeatureVal* pfeatures/*may be NULL*/)
{                      //When finished with the Features, call features_destroy    
    if (pfeatures)
    return pfeatures->clone();
    else
    return new Features;
}

#if 0
//not public since there is no public way of making the mask
GRNG_EXPORT bool features_masked_or(Features* pSrc, const Features* pOther, const Features* pMask)    //returns false iff any of the Features* are NULL
{
    if (!pSrc)
    return false;
    if (!pOther)
    return false;
    if (!pMask)
    return false;
    
    pSrc->maskedOr(*pOther, *pMask);
    return true;
}
#endif 
  
GRNG_EXPORT void gr_featureval_destroy(GrFeatureVal *p)
{
    delete p;
}



}}}} // namespace
