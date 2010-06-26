#include "graphiteng/FeaturesHandle.h"
#include "Features.h"

GRNG_EXPORT void DeleteFeatures(Features *p)
{
    delete p;
}


FeaturesHandle FeaturesHandle::clone() const		//clones the Features which are then owned separately
{
    if (Ptr())
	return Ptr()->clone();
    else
	return FeaturesHandle();
}


bool FeaturesHandle::maskedOr(const FeaturesHandle& other, const FeaturesHandle& mask) const	//returns false iff any of the FeaturesHandles are IsNull
{
    if (IsNull())
	return false;
    if (other.IsNull())
	return false;
    if (mask.IsNull())
	return false;
    
    Ptr()->maskedOr(*other.Ptr(), *mask.Ptr());
    return true;
}

