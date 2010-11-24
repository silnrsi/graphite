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
#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class Features;     //the set of Features that are supported for a particular face. 
              //Different languages or users may prefer different combinations of Features of the face to be enabled.
              //Internally, in the face, a feature is named by an identifier which is a 32-bit number which can take the form of a tag (4 characters)
              //or if the top byte is 0 a number.
              //But for speed of use within Graphite, the identifier is converted to a FeatureRef to enable the face's features to be manipulated more efficiently.



extern "C"
{
    GRNG_EXPORT Features* features_clone(const Features* pfeatures/*may be NULL*/);
                      //When finished with the Features, call features_destroy    

    GRNG_EXPORT bool features_masked_or(Features* pSrc, const Features* pOther, const Features* pMask);    //returns false iff any of the Features* are NULL

    GRNG_EXPORT void features_destroy(Features *pfeatures);
}



}}}}
