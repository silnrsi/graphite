#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class Features;


extern "C"
{
    GRNG_EXPORT Features* make_empty_Features();
                      //When finished with the Features, call destroy_Features    
    GRNG_EXPORT Features* clone_Features(const Features* pfeatures);
                      //When finished with the Features, call destroy_Features    

    GRNG_EXPORT bool maskedOr(Features* pSrc, const Features* pOther, const Features* pMask);    //returns false iff any of the Features* are NULL

    GRNG_EXPORT void destroy_Features(Features *pfeatures);
}



}}}}
