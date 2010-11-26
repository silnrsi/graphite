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

class FeatureRef;
class Features;
class GrFace;
/*  A FeatureRef provides a handle for efficient access of settings from a set of values called Features.
    Both the FeatureRef and the Features must be associated with the same face.
*/


extern "C"
{
    GRNG_EXPORT FeatureRef* make_FeatureRef(byte bits, byte index, uint32 mask);
                      //When finished with the FeatureRef, call destroy_FeatureRef    
    GRNG_EXPORT FeatureRef* clone_FeatureRef(const FeatureRef*pfeatureref);
                      //When finished with the FeatureRef, call destroy_FeatureRef    

    GRNG_EXPORT uint16 fref_max_value(const FeatureRef* pfeatureref);    //returns 0 if pointer is NULL
    GRNG_EXPORT uint16 fref_feature_value(const FeatureRef*pfeatureref, const Features* feats);    //returns 0 if either pointer is NULL
    GRNG_EXPORT bool fref_set_feature_value(const FeatureRef* pfeatureref, uint16 val, Features* pDest);    //returns false iff either pointer is NULL. or if they are not for the same face, or val is too big
    GRNG_EXPORT uint32 fref_id(const FeatureRef*pfeatureref);

    // Labels may not be available for requested langId, the language actually used
    // will be returned in langId. The length in bytes will be returned in length
    // call destroy_feature_label when finished
    GRNG_EXPORT void* feature_label(const GrFace* pFace, const FeatureRef*pfeatureref, uint16 *langId, encform utf, uint32 *length);
    GRNG_EXPORT uint16 num_feature_settings(const FeatureRef*pfeatureref);
    GRNG_EXPORT int16 feature_setting_value(const FeatureRef*pfeatureref, uint16 setting);
    GRNG_EXPORT void* feature_setting_label(const GrFace* pFace, const FeatureRef*pfeatureref, uint16 setting, uint16 *langId, encform utf, uint32 *length);
    GRNG_EXPORT void destroy_feature_label(void * label);

    GRNG_EXPORT void destroy_FeatureRef(FeatureRef *pfeatureref);
}




}}}}
