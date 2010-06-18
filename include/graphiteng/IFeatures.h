#ifndef IFEATURES_INCLUDE
#define IFEATURES_INCLUDE

#include "graphiteng/Types.h"

class FeatureRef;

class IFeatures
{
public:
    virtual void addFeature(FeatureRef &ref, uint16 value) = 0;
    virtual uint16 getFeature(FeatureRef &ref) = 0;
    virtual IFeatures *newCopy() = 0;
};

#endif
