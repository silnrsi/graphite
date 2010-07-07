#include "graphiteng/IFont.h"
#include "GrFont.h"


GrFont* IFont::makeGrFont(const LoadedFace *face) const			//this must stay alive all the time when the GrFont is alive. When finished with the GrFont, call IFont::destroyLoadedFace
{
    return new GrHintedFont(this, face);
}


/*static*/ GrFont* IFont::makeGrFont(float ppm, const LoadedFace *face)		//When finished with the GrFont, call IFont::destroyLoadedFace
{
    return new GrFont(ppm, face);
}


/*static*/ void IFont::destroyGrFont(GrFont *font)
{
    delete font;
}




