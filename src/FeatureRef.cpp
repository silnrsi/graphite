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
#include "graphiteng/Font.h"
#include "FeatureMap.h"
#include "GrFaceImp.h"
#include "NameTable.h"

using namespace org::sil::graphite::v2;

namespace org { namespace sil { namespace graphite { namespace v2 {
/*
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
*/

GRNG_EXPORT uint16 fref_feature_value(const FeatureRef*pfeatureref, const Features* feats)    //returns 0 if either pointer is NULL
{
    if (!pfeatureref)
    return 0;
    if (!feats)
    return 0;
    
    return pfeatureref->getFeatureVal(*feats);
}


GRNG_EXPORT int fref_set_feature_value(const FeatureRef* pfeatureref, uint16 val, Features* pDest)
{
    if (!pfeatureref)
    return false;
    if (!pDest)
    return false;
    
    return pfeatureref->applyValToFeature(val, pDest);
}


GRNG_EXPORT uint32 fref_id(const FeatureRef* pfeatureref)    //returns 0 if pointer is NULL
{
  if (!pfeatureref)
    return 0;
  
  return pfeatureref->getId();
}


GRNG_EXPORT uint16 fref_n_values(const FeatureRef*pfeatureref)
{
    if(!pfeatureref)
        return 0;
    return pfeatureref->getNumSettings();
}


GRNG_EXPORT int16 fref_value(const FeatureRef*pfeatureref, uint16 settingno)
{
    if(!pfeatureref || (settingno >= pfeatureref->getNumSettings()))
    {
        return 0;
    }
    return pfeatureref->getSettingValue(settingno);
}


GRNG_EXPORT void* fref_label(const FeatureRef*pfeatureref, uint16 *langId, encform utf, uint32 *length)
{
    if(!pfeatureref || !pfeatureref->getFace())
    {
        langId = 0;
        length = 0;
        return NULL;
    }
    uint16 label = pfeatureref->getNameId();
    NameTable * names = pfeatureref->getFace()->nameTable();
    if (!names)
    {
        langId = 0;
        length = 0;
        return NULL;
    }
    return names->getName(*langId, label, utf, *length);
}


GRNG_EXPORT void* fref_value_label(const FeatureRef*pfeatureref, uint16 setting, uint16 *langId, encform utf,
    uint32 *length)
{
    if(!pfeatureref || (setting >= pfeatureref->getNumSettings()) || !pfeatureref->getFace())
    {
        langId = 0;
        length = 0;
        return NULL;
    }
    uint16 label = pfeatureref->getSettingName(setting);
    NameTable * names = pfeatureref->getFace()->nameTable();
    if (!names)
    {
        langId = 0;
        length = 0;
        return NULL;
    }
    return names->getName(*langId, label, utf, *length);
}


GRNG_EXPORT void label_destroy(void * label)
{
    if (label)
        free(label);
}

}}}} // namespace
