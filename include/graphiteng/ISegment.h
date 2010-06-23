#ifndef ISEGMENT_INCLUDE
#define ISEGMENT_INCLUDE

#include "graphiteng/Types.h"

class ISlot;

class ISegment 
{
public:
    virtual int length() const = 0;
    virtual Position advance() const = 0;
    virtual ISlot & operator[] (int index) = 0;
    virtual const ISlot & operator[] (int index) const = 0;
    virtual void runGraphite() = 0;
    virtual void chooseSilf(uint32 script) = 0;
};

class IFace;
class FontFace;

extern GRNG_EXPORT FontFace *create_fontface(IFace *face);
extern GRNG_EXPORT void destroy_fontface(FontFace *face);

#ifndef DISABLE_FILE_FONT
extern GRNG_EXPORT FontFace *create_filefontface(const char * filePath);
#endif



class IFont;
class FontImpl;

extern GRNG_EXPORT FontImpl *create_font(IFont *font, FontFace *face, float ppm);
extern GRNG_EXPORT void destroy_font(FontImpl *font);



class ITextSource;

extern GRNG_EXPORT ISegment *create_rangesegment(FontImpl *font, FontFace *face, const ITextSource *txt);
extern GRNG_EXPORT void destroy_segment(ISegment *seg);

#endif // SEGMENT_INCLUDE
