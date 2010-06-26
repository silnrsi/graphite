#include "graphiteng/FeatureRefHandle.h"
#include "FeatureMap.h"

GRNG_EXPORT void DeleteFeatureRef(FeatureRef *p)
{
    delete p;
}


FeatureRefHandle::FeatureRefHandle(byte bits/*=0*/, byte index/*=0*/, uint32 mask/*=0*/)
:	AutoHandle<FeatureRef, &DeleteFeatureRef>(new FeatureRef(bits, index, mask))
{
}


FeatureRefHandle FeatureRefHandle::clone() const		//clones the FeatureRef which are then owned separately
{
    if (Ptr())
	return new FeatureRef(*Ptr());
    else
	return FeatureRefHandle();
}


void FeatureRefHandle::applyValToFeature(uint16 val, const FeaturesHandle& pDest) const
{
    if (IsNull())
	return;
    if (pDest.IsNull())
	return;
    
    Ptr()->applyValToFeature(val, pDest.Ptr());
}


void FeatureRefHandle::maskFeature(const FeaturesHandle& pDest) const
{
    if (IsNull())
	return;
    if (pDest.IsNull())
	return;
    
    Ptr()->maskFeature(pDest.Ptr());
}


uint16 FeatureRefHandle::getFeatureVal(const FeaturesHandle& feats) const	//returns 0 if either handle IsNull
{
    if (IsNull())
	return 0;
    if (feats.IsNull())
	return 0;
    
    Ptr()->getFeatureVal(*feats.Ptr());
}

