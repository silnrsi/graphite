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
#include "FeatureMap.h"
#include "Face.h"
#include "NameTable.h"

extern "C"
{

GRNG_EXPORT gr_uint16 gr_fref_feature_value(const gr_feature_ref* pfeatureref, const gr_feature_val* feats)    //returns 0 if either pointer is NULL
{
    if (!pfeatureref)
    return 0;
    if (!feats)
    return 0;
    
    return pfeatureref->getFeatureVal(*feats);
}


GRNG_EXPORT int gr_fref_set_feature_value(const gr_feature_ref* pfeatureref, gr_uint16 val, gr_feature_val* pDest)
{
    if (!pfeatureref)
    return false;
    if (!pDest)
    return false;
    
    return pfeatureref->applyValToFeature(val, pDest);
}


GRNG_EXPORT uint32 gr_fref_id(const gr_feature_ref* pfeatureref)    //returns 0 if pointer is NULL
{
  if (!pfeatureref)
    return 0;
  
  return pfeatureref->getId();
}


GRNG_EXPORT uint16 gr_fref_n_values(const gr_feature_ref* pfeatureref)
{
    if(!pfeatureref)
        return 0;
    return pfeatureref->getNumSettings();
}


GRNG_EXPORT int16 gr_fref_value(const gr_feature_ref* pfeatureref, uint16 settingno)
{
    if(!pfeatureref || (settingno >= pfeatureref->getNumSettings()))
    {
        return 0;
    }
    return pfeatureref->getSettingValue(settingno);
}


GRNG_EXPORT void* gr_fref_label(const gr_feature_ref* pfeatureref, uint16 *langId, gr_encform utf, uint32 *length)
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


GRNG_EXPORT void* gr_fref_value_label(const gr_feature_ref*pfeatureref, uint16 setting,
    uint16 *langId, gr_encform utf, uint32 *length)
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


GRNG_EXPORT void gr_label_destroy(void * label)
{
    if (label)
        free(label);
}

} // extern "C"