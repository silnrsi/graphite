#ifndef ITEXTSOURCE_INCLUDE
#define ITEXTSOURCE_INCLUDE

#include <stddef.h>

#define FIND_BROKEN_VIRTUALS

class ITextSource
{
public :
    enum encform {
      kutf8 = 1/*sizeof(uint8)*/, kutf16 = 2/*sizeof(uint16)*/, kutf32 = 4/*sizeof(uint32)*/
    };
 
    virtual encform utfEncodingForm() const = 0;
    virtual size_t getLength() const = 0;		//number of unicode characters i.e. code points
    virtual const void* get_utf_buffer_begin() const = 0;  
    
private :
#ifdef FIND_BROKEN_VIRTUALS
    virtual double utfEncodingForm() {return 0.0;}
    virtual double getLength() {return 0.0;}		//number of unicode characters i.e. code points
    virtual double get_utf8_buffer() {return 0.0;}
    virtual double get_utf16_buffer() {return 0.0;}
    virtual double get_utf32_buffer() {return 0.0;}
#endif		//FIND_BROKEN_VIRTUALS
};

#endif
