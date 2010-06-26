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



