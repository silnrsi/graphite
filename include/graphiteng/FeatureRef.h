#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class FeatureRef;
class Features;

/*  A FeatureRef provides a handle for efficient access of settings from a set of values called Features.
    Both the FeatureRef and the Features must be associated with the same face.
*/


extern "C"
{
    GRNG_EXPORT uint32 fref_name(const FeatureRef* pfeatureref);    //returns 0 if pointer is NULL
    GRNG_EXPORT uint16 fref_max_value(const FeatureRef* pfeatureref);    //returns 0 if pointer is NULL
    GRNG_EXPORT uint16 fref_feature_value(const FeatureRef* pfeatureref, const Features* feats);    //returns 0 if either pointer is NULL. or if they are not for the same face
    GRNG_EXPORT bool fref_set_feature_value(const FeatureRef* pfeatureref, uint16 val, Features* pDest);    //returns false iff either pointer is NULL. or if they are not for the same face, or val is too big
}




}}}}
