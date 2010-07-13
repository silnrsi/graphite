#include "graphiteng/FeatureRefHandle.h"
#include "FeatureMap.h"

using namespace org::sil::graphite::v2;

namespace org { namespace sil { namespace graphite { namespace v2 {

GRNG_EXPORT void DeleteFeatureRef(FeatureRef *p)
{
    delete p;
}

}}}} // namespace

FeatureRefHandle::FeatureRefHandle(byte bits, byte index, uint32 mask/*=0*/)
:	AutoHandle<FeatureRef, &DeleteFeatureRef>(new FeatureRef(bits, index, mask))
{
}


FeatureRefHandle FeatureRefHandle::clone() const		//clones the FeatureRef which are then owned separately
{
    if (ptr())
	return new FeatureRef(*ptr());
    else
	return NULL;
}


void FeatureRefHandle::applyValToFeature(uint16 val, const FeaturesHandle& pDest) const
{
    if (isNull())
	return;
    if (pDest.isNull())
	return;
    
    ptr()->applyValToFeature(val, pDest.ptr());
}


void FeatureRefHandle::maskFeature(const FeaturesHandle& pDest) const
{
    if (isNull())
	return;
    if (pDest.isNull())
	return;
    
    ptr()->maskFeature(pDest.ptr());
}


uint16 FeatureRefHandle::getFeatureVal(const FeaturesHandle& feats) const	//returns 0 if either handle IsNull
{
    if (isNull())
	return 0;
    if (feats.isNull())
	return 0;
    
    return ptr()->getFeatureVal(*feats.ptr());
}

