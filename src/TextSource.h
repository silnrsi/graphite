#ifndef TEXTSOURCE_INCLUDE
#define TEXTSOURCE_INCLUDE

enum encform {
    kutf8 = 1, kutf16 = 2, kutf32 = 3
};

class TextSource
{
public :
    virtual encform utfEncodingForm() = 0;
    virtual int getLength() = 0;
    virtual unsigned char *get_utf8_buffer() = 0;
    virtual unsigned short *get_utf16_buffer() = 0;
    virtual unsigned long *get_utf32_buffer() = 0;
};

#endif
