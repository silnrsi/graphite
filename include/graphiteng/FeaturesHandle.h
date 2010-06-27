#ifndef FEATURES_HANDLE_INCLUDE
#define FEATURES_HANDLE_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/AutoHandle.h"

class Features;

extern GRNG_EXPORT void DeleteFeatures(Features *p);

class GRNG_EXPORT FeaturesHandle : public AutoHandle<Features, &DeleteFeatures>
{
public:
    FeaturesHandle() {}
    FeaturesHandle(Features* p/*takes ownership*/) : AutoHandle<Features, &DeleteFeatures>(p) {}
    
    FeaturesHandle clone() const;		//clones the Features which is then owned separately
    bool maskedOr(const FeaturesHandle& other, const FeaturesHandle& mask) const;	//returns false iff any of the FeaturesHandles are IsNull

private:
    friend class Segment;
    friend class FeatureMap;
    friend class FeatureRefHandle;
    friend class SegmentHandle;
    Features* ptr() const { return AutoHandle<Features, &DeleteFeatures>::ptr(); }
};


#endif // !FEATURES_HANDLE_INCLUDE
