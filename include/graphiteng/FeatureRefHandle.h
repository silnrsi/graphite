#ifndef FEATURE_REF_HANDLE_INCLUDE
#define FEATURE_REF_HANDLE_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/AutoHandle.h"

class FeatureRef;
class FeaturesHandle;

extern GRNG_EXPORT void DeleteFeatureRef(FeatureRef *p);

class GRNG_EXPORT FeatureRefHandle : public AutoHandle<FeatureRef, &DeleteFeatureRef>
{
public:
    FeatureRefHandle(byte bits, byte index, uint32 mask=0);
    FeatureRefHandle(FeatureRef* p/*takes ownership*/) : AutoHandle<FeatureRef, &DeleteFeatureRef>(p) {}
    
    FeatureRefHandle clone() const;		//clones the FeatureRef which is then owned separately

    void applyValToFeature(uint16 val, const FeaturesHandle& pDest) const;
    void maskFeature(const FeaturesHandle& pDest) const;
    uint16 getFeatureVal(const FeaturesHandle& feats) const;	//returns 0 if either handle IsNull

private:
//    friend class Segment;
//    friend class FeatureMap;
//    FeatureRef* Ptr() const { return AutoHandle<FeatureRef, &DeleteFeatureRef>::Ptr(); }
};


#endif // !FEATURE_REF_HANDLE_INCLUDE
