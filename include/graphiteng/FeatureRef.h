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


extern "C"
{
    GRNG_EXPORT FeatureRef* make_FeatureRef(byte bits, byte index, uint32 mask);
                      //When finished with the FeatureRef, call destroy_FeatureRef    
    GRNG_EXPORT FeatureRef* clone_FeatureRef(const FeatureRef*pfeatureref);
                      //When finished with the FeatureRef, call destroy_FeatureRef    

    GRNG_EXPORT void apply_value_to_feature(uint16 val, FeatureRef* pRes, Features* pDest);
    GRNG_EXPORT void mask_feature(const FeatureRef* pfeatureref, Features* pDest);
    GRNG_EXPORT uint16 feature_value(const FeatureRef*pfeatureref, const Features* feats);    //returns 0 if either pointer is NULL
    GRNG_EXPORT uint32 feature_id(const FeatureRef*pfeatureref);

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
