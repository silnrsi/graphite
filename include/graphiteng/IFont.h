#ifndef IFONT_INCLUDE
#define IFONT_INCLUDE

#include "graphiteng/Types.h"

class LoadedFace;
class LoadedFont;

class GRNG_EXPORT IFont
{
public:
    virtual float advance(uint16 glyphid) const = 0;		//amount to advance. positive is in the writing direction
    virtual float ppm() const = 0;				//pixels per em
    
    LoadedFont* makeLoadedFont(const LoadedFace *face) const;				//the 'this' must stay alive all the time when the LoadedFont is alive. When finished with the LoadedFont, call IFont::destroyLoadedFace
    static LoadedFont* makeLoadedFont(float ppm, const LoadedFace *face);		//When finished with the LoadedFont, call IFont::destroyLoadedFace
    static void destroyLoadedFont(LoadedFont *font);
    
private :
#ifdef FIND_BROKEN_VIRTUALS
    virtual void advance(uint16 glyphid) {}
    virtual void ppm() {}
#endif		//FIND_BROKEN_VIRTUALS
};





#endif // FONT_INCLUDE
