#ifndef TEXTSOURCE_INCLUDE
#define TEXTSOURCE_INCLUDE

enum encform {
    kutf8 = 1, kutf16 = 2, kutf32 = 3
};

class TextSource
{
public :
    encform utfEncodingForm();
    int getLength();
    unsigned char *get_utf8_buffer();
    unsigned short *get_utf16_buffer();
    unsigned long *get_utf32_buffer();
};

#endif
