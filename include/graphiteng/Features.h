#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class Features;     //the set of Features that are supported for a particular face. 
              //Different languages or users may prefer different combinations of Features of the face to be enabled.
              //Internally, in the face, a feature is named by an identifier which is a 32-bit number which can take the form of a tag (4 characters)
              //or if the top byte is 0 a number.
              //But for speed of use within Graphite, the identifier is converted to a FeatureRef to enable the face's features to be manipulated more efficiently.



extern "C"
{
    GRNG_EXPORT Features* features_clone(const Features* pfeatures/*may be NULL*/);
                      //When finished with the Features, call features_destroy    

    GRNG_EXPORT bool features_masked_or(Features* pSrc, const Features* pOther, const Features* pMask);    //returns false iff any of the Features* are NULL

    GRNG_EXPORT void features_destroy(Features *pfeatures);
}



}}}}
