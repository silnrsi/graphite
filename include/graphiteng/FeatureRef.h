#pragma once

#include "graphiteng/Types.h"
#include "graphiteng/AutoHandle.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class FeatureRef;
class FeaturesHandle;


extern "C"
{
    GRNG_EXPORT FeatureRef* make_FeatureRef(byte bits, byte index, uint32 mask);
                      //When finished with the FeatureRef, call destroy_FeatureRef    
    GRNG_EXPORT FeatureRef* clone_FeatureRef(const FeatureRef*pfeatureref);
                      //When finished with the FeatureRef, call destroy_FeatureRef    

    GRNG_EXPORT void apply_value_to_feature(uint16 val, const FeaturesHandle& pDest, FeatureRef* pRes);
    GRNG_EXPORT void mask_feature(const FeatureRef* pfeatureref, const FeaturesHandle& pDest);
    GRNG_EXPORT uint16 get_feature_value(const FeatureRef*pfeatureref, const FeaturesHandle& feats);    //returns 0 if either pointer is NULL

    GRNG_EXPORT void destroy_FeatureRef(FeatureRef *pfeatureref);
}




}}}}
