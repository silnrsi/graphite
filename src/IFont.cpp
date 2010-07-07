#include "graphiteng/IFont.h"
#include "LoadedFont.h"


LoadedFont* IFont::makeLoadedFont(const LoadedFace *face) const			//this must stay alive all the time when the LoadedFont is alive. When finished with the LoadedFont, call IFont::destroyLoadedFace
{
    return new LoadedFontWithHints(this, face);
}


/*static*/ LoadedFont* IFont::makeLoadedFont(float ppm, const LoadedFace *face)		//When finished with the LoadedFont, call IFont::destroyLoadedFace
{
    return new LoadedFont(ppm, face);
}


/*static*/ void IFont::destroyLoadedFont(LoadedFont *font)
{
    delete font;
}




