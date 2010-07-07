#include "graphiteng/FeaturesHandle.h"
#include "Features.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

GRNG_EXPORT void DeleteFeatures(Features *p)
{
    delete p;
}


FeaturesHandle FeaturesHandle::clone() const		//clones the Features which are then owned separately
{
    if (ptr())
	return ptr()->clone();
    else
	return FeaturesHandle();
}


bool FeaturesHandle::maskedOr(const FeaturesHandle& other, const FeaturesHandle& mask) const	//returns false iff any of the FeaturesHandles are IsNull
{
    if (isNull())
	return false;
    if (other.isNull())
	return false;
    if (mask.isNull())
	return false;
    
    ptr()->maskedOr(*other.ptr(), *mask.ptr());
    return true;
}

}}}} // namespace
