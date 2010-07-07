#pragma once

#include "graphiteng/Types.h"

class GrFace;
class GrFont;

class GRNG_EXPORT IFont
{
public:
    virtual float advance(uint16 glyphid) const = 0;		//amount to advance. positive is in the writing direction
    virtual float ppm() const = 0;				//pixels per em
    
    GrFont* makeGrFont(const GrFace *face) const;				//the 'this' must stay alive all the time when the GrFont is alive. When finished with the GrFont, call IFont::destroyGrFont
    static GrFont* makeGrFont(float ppm, const GrFace *face);		//When finished with the GrFont, call IFont::destroyGrFont
    static void destroyGrFont(GrFont *font);
    
private :
#ifdef FIND_BROKEN_VIRTUALS
    virtual void advance(uint16 glyphid) {}
    virtual void ppm() {}
#endif		//FIND_BROKEN_VIRTUALS
};





