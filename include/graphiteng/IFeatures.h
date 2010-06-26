#ifndef IFEATURES_INCLUDE
#define IFEATURES_INCLUDE

#include "graphiteng/Types.h"

class FeatureRef;
class Features;

class GRNG_EXPORT IFeatures
{
public:
    void addFeature(FeatureRef *ref, uint16 value) = 0;
    uint16 getFeature(FeatureRef *ref) = 0;
    IFeatures *newCopy() = 0;

private:
    Features *m_feats;
};

#endif
