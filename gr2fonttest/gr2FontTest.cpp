/*-----------------------------------------------------------------------------
Copyright (C) 2005 www.thanlwinsoft.org

Distributable under the terms of either the Common Public License or the
GNU Lesser General Public License, as specified in the LICENSING.txt file.

File: 
Responsibility: Keith Stribley
Last reviewed: Not yet.

Description:
A simple console app that creates a segment using FileFont and dumps a 
diagnostic table of the resulting glyph vector to the console. 
If graphite has been built with -DTRACING then it will also produce a
diagnostic log of the segment creation in grSegmentLog.txt
-----------------------------------------------------------------------------*/

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <iomanip>
#include <cstring>
#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#include "graphite2/Types.h"
#include "graphite2/Segment.h"
#include "graphite2/XmlLog.h"

#if !defined WORDS_BIGENDIAN || defined PC_OS
#define swap16(x) (((x & 0xff) << 8) | ((x & 0xff00) >> 8))
#define swap32(x) (((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x & 0xff000000) >> 24))
#else
#define swap16(x) (x)
#define swap32(x) (x)
#endif

class Gr2TextSrc
{

public:
    Gr2TextSrc(const gr_uint32* base, size_t len) : m_buff(base), m_len(len) { }
    gr_encform utfEncodingForm() const { return gr_utf32; }
    size_t getLength() const { return m_len; }
    const void* get_utf_buffer_begin() const { return m_buff; }

private:
    const gr_uint32* m_buff;
    size_t m_len;
};

#ifndef HAVE_STRTOF
float strtof(char * text, char ** /*ignore*/)
{
  return static_cast<float>(atof(text));
}
#endif

#ifndef HAVE_STRTOL
long strtol(char * text, char ** /*ignore*/)
{
  return atol(text);
}
#endif

class Parameters
{
public:
    Parameters();
    ~Parameters();
    void clear();
    void closeLog();
    bool loadFromArgs(int argc, char *argv[]);
    int testFileFont() const;
    gr_feature_val* parseFeatures(const gr_face * face) const;
    void printFeatures(const gr_face * face) const;
public:
    const char * fileName;
    const char * features;
    float pointSize;
    int dpi;
    bool lineStart;
    bool lineEnd;
    bool ws;
    bool rtl;
    bool useLineFill;
    bool useCodes;
    bool justification;
    bool enableCache;
    float width;
    int textArgIndex;
    unsigned int * pText32;
    size_t charLength;
    size_t offset;
    FILE * log;
    FILE * trace;
    int mask;
    
private :  //defensive since log should not be copied
    Parameters(const Parameters&);
    Parameters& operator=(const Parameters&);
};

Parameters::Parameters()
{
  log = stdout ;
  clear();
}


Parameters::~Parameters()
{
  free(pText32);
  pText32 = NULL;
  closeLog();
}

void Parameters::clear()
{
    closeLog() ;
    fileName = "";
    pointSize = 12.0f;
    dpi = 72;
    lineStart = false;
    lineEnd = false;
    rtl = false;
    ws = false;
    useLineFill = false;
    useCodes = false;
    justification = false;
    enableCache = false;
    width = 100.0f;
    pText32 = NULL;
    textArgIndex = 0;
    charLength = 0;
    offset = 0;
    log = stdout;
    trace = NULL;
    mask = GRLOG_SEGMENT | GRLOG_OPCODE;
}


void Parameters::closeLog()
{
  if (log==stdout)
    return ;
  
  fclose(log);
  log = stdout;
}

int lookup(size_t *map, size_t val);


#ifdef HAVE_ICONV

template <class A,class B> int
convertUtf(const char * inType, const char * outType, A* pIn, B * & pOut)
{
    int length = 0;//strlen(reinterpret_cast<char*>(pIn));
    while (pIn[length] != 0) length++;
    int outFactor = 1;
    if (sizeof(B) < sizeof(A))
    {
        outFactor = sizeof(A) / sizeof(B);
    }
#ifdef WIN32
    const char * pText = reinterpret_cast<char*>(pIn);
#else
    char * pText = reinterpret_cast<char*>(pIn);
#endif
    // It seems to be necessary to include the trailing null to prevent
    // stray characters appearing with utf16
    size_t bytesLeft = (length+1) * sizeof(A);
    // allow 2 extra for null + bom
    size_t outBytesLeft = (length*outFactor + 2) * sizeof(B);
    size_t outBufferSize = outBytesLeft;
    //B * textOut = new B[length*outFactor + 2];
    B * textOut = reinterpret_cast<B*>(malloc(sizeof(B) * length*outFactor + 2));// new operator not defined
    iconv_t utfInOut = iconv_open(outType,inType);
    assert(utfInOut != (iconv_t)(-1));
    char * pTextOut = reinterpret_cast<char*>(&textOut[0]);
    size_t status = iconv(utfInOut, &pText, &bytesLeft, &pTextOut, &outBytesLeft);
    if (status == size_t(-1)) perror("iconv failed:");
    size_t charLength = (outBufferSize - outBytesLeft) / sizeof(B);
    assert(status != size_t(-1));
    // offset by 1 to avoid bom
    unsigned char * bom = reinterpret_cast<unsigned char*>(&textOut[0]);
    if ((charLength * sizeof(B) > 1) && ((bom[1] == 0xfe && bom[0] == 0xff)
        || (bom[0] == 0xfe && bom[1] == 0xff)))
    {
            charLength--;
            for (size_t i = 0; i < charLength; i++)
                textOut[i] = textOut[i+1];
            textOut[charLength] = 0;
    }
    if (textOut[charLength] == 0) --charLength;
    pOut = textOut;
    iconv_close(utfInOut);
    return charLength;
}

#endif

bool Parameters::loadFromArgs(int argc, char *argv[])
{
    int mainArgOffset = 0;
    pText32 = NULL;
    features = NULL;
    log = stdout;
    bool argError = false;
    char* pText = NULL;
    typedef enum 
    {
        NONE,
        POINT_SIZE,
        DPI,
        LINE_START,
        LINE_END,
        LINE_FILL,
        CODES,
        FEAT,
        LOG,
        TRACE,
        TRACE_MASK
    } TestOptions;
    TestOptions option = NONE;
    char * pIntEnd = NULL;
    char * pFloatEnd = NULL;
    long lTestSize = 0;
    float fTestSize = 0.0f;
    for (int a = 1; a < argc; a++)
    {
        switch (option)
        {
        case DPI:
            pIntEnd = NULL;
            lTestSize = strtol(argv[a],&pIntEnd, 10);
            if (lTestSize > 0 && lTestSize < INT_MAX && lTestSize != LONG_MAX)
            {
                dpi = lTestSize;
            }
            else
            {
                fprintf(stderr,"Invalid dpi %s\n", argv[a]);
            }
            option = NONE;
            break;
        case POINT_SIZE:
            pFloatEnd = NULL;
            fTestSize = strtof(argv[a],&pFloatEnd);
            // what is a reasonable maximum here
            if (fTestSize > 0 && fTestSize < 5000.0f)
            {
                pointSize = fTestSize;
            }
            else
            {
                fprintf(stderr,"Invalid point size %s\n", argv[a]);
                argError = true;
            }
            option = NONE;
            break;
        case LINE_FILL:
            pFloatEnd = NULL;
            fTestSize = strtof(argv[a],&pFloatEnd);
            // what is a good max width?
            if (fTestSize > 0 && fTestSize < 10000)
            {
                width = fTestSize;
            }
            else
            {
                fprintf(stderr,"Invalid line width %s\n", argv[a]);
                argError = true;
            }
            option = NONE;
            break;
        case FEAT:
                features = argv[a];
                option = NONE;
                break;
        case LOG:
            closeLog();
            log = fopen(argv[a], "w");
            if (log == NULL)
            {
                fprintf(stderr,"Failed to open %s\n", argv[a]);
                log = stdout;
            }
            option = NONE;
            break;
        case TRACE:
            if (trace) fclose(trace);
            trace = fopen(argv[a], "wb");
            if (trace == NULL)
            {
                fprintf(stderr,"Failed to open %s\n", argv[a]);
            }
            option = NONE;
            break;
        case TRACE_MASK:
            mask = atoi(argv[a]);
            option = NONE;
            break;
        default:
            option = NONE;
            if (argv[a][0] == '-')
            {
                if (strcmp(argv[a], "-pt") == 0)
                {
                    option = POINT_SIZE;
                }
                else if (strcmp(argv[a], "-dpi") == 0)
                {
                    option = DPI;
                }
                else if (strcmp(argv[a], "-ls") == 0)
                {
                    option = NONE;
                    lineStart = true;
                }
                else if (strcmp(argv[a], "-le") == 0)
                {
                    option = NONE;
                    lineEnd = true;
                }
                else if (strcmp(argv[a], "-le") == 0)
                {
                    option = NONE;
                    lineEnd = true;
                }
                else if (strcmp(argv[a], "-rtl") == 0)
                {
                    option = NONE;
                    rtl = true;
                }
                else if (strcmp(argv[a], "-ws") == 0)
                {
                    option = NONE;
                    ws = true;
                }
                else if (strcmp(argv[a], "-cache") == 0)
                {
                    option = NONE;
                    enableCache = true;
                }
                else if (strcmp(argv[a], "-feat") == 0)
                {
                    option = FEAT;
                }
                else if (strcmp(argv[a], "-codes") == 0)
                {
                    option = NONE;
                    useCodes = true;
                    // must be less than argc
                    //pText32 = new unsigned int[argc];
                    pText32 = (unsigned int *)malloc(sizeof(unsigned int) * argc);
                    fprintf(log, "Text codes\n");
                }
                else if (strcmp(argv[a], "-linefill") == 0)
                {
                    option = LINE_FILL;
                    useLineFill = true;
                }
                else if (strcmp(argv[a], "-j") == 0)
                {
                    option = NONE;
                    justification = true;
                }
                else if (strcmp(argv[a], "-log") == 0)
                {
                    option = LOG;
                }
                else if (strcmp(argv[a], "-trace") == 0)
                {
                    option = TRACE;
                }
                else if (strcmp(argv[a], "-mask") == 0)
                {
                    option = TRACE_MASK;
                }
                else
                {
                    argError = true;
                    fprintf(stderr,"Unknown option %s\n",argv[a]);
                }
            }
            else if (mainArgOffset == 0)
            {
                fileName = argv[a];
                mainArgOffset++;
            }
            else if (useCodes)
            {
                pIntEnd = NULL;
                mainArgOffset++;
                unsigned int code = strtol(argv[a],&pIntEnd, 16);
                if (code > 0)
                {
// convert text to utfOut using iconv because its easier to debug string placements
                    pText32[charLength++] = code;
                    if (charLength % 10 == 0)
                        fprintf(log, "%4x\n",code);
                    else
                        fprintf(log, "%4x\t",code);
                }
                else
                {
                    fprintf(stderr,"Invalid dpi %s\n", argv[a]);
                }
            }
            else if (mainArgOffset == 1)
            {
                mainArgOffset++;
                pText = argv[a];
                textArgIndex = a;
            }
            else
            {
                argError = true;
                fprintf(stderr,"too many arguments %s\n",argv[a]);
            }
        }
    }
    if (mainArgOffset < 1) argError = true;
    else if (mainArgOffset > 1)
    {
        if (!useCodes && pText != NULL)
        {
#ifdef HAVE_ICONV
            charLength = convertUtf("utf-8","utf-32",pText, pText32);
            fprintf(log, "String has %d characters\n", (int)charLength);
            size_t ci;
            for (ci = 0; ci < 10 && ci < charLength; ci++)
            {
                    fprintf(log, "%d\t", (int)ci);
            }
            fprintf(log, "\n");
            for (ci = 0; ci < charLength; ci++)
            {
                    fprintf(log, "%04x\t", (int)ci);
                    if (((ci + 1) % 10) == 0)
                        fprintf(log, "\n");
            }
            fprintf(log, "\n");
#else
            fprintf(stderr,"Only the -codes option is supported on Win32\n");
            argError = true;
#endif
        }
        else 
        {
            pText32[charLength] = 0;
            fprintf(log, "\n");
        }
    }
    return (argError) ? false : true;
}

union FeatID
{
    gr_uint8 uChar[4];
    gr_uint32 uId;
};

void Parameters::printFeatures(const gr_face * face) const
{
    gr_uint16 numFeatures = gr_face_n_fref(face);
    fprintf(log, "%d features\n", numFeatures);
    gr_uint16 langId = 0x0409;
    for (gr_uint16 i = 0; i < numFeatures; i++)
    {
        const gr_feature_ref * f = gr_face_fref(face, i);
        gr_uint32 length = 0;
        char * label = reinterpret_cast<char *>(gr_fref_label(f, &langId, gr_utf8, &length));
        FeatID featId;
        featId.uId = gr_fref_id(f);
        if (label)
            if ((featId.uChar[0] >= 0x20 && featId.uChar[0] < 0x7F) &&
                (featId.uChar[1] >= 0x20 && featId.uChar[1] < 0x7F) &&
                (featId.uChar[2] >= 0x20 && featId.uChar[2] < 0x7F) &&
                (featId.uChar[3] >= 0x20 && featId.uChar[3] < 0x7F))
            {
#ifdef WORDS_BIGENDIAN
                fprintf(log, "%d %c%c%c%c %s\n", featId.uId, featId.uChar[0], featId.uChar[1],
                       featId.uChar[2], featId.uChar[3], label);
#else
                fprintf(log, "%d %c%c%c%c %s\n", featId.uId, featId.uChar[3], featId.uChar[2],
                       featId.uChar[1], featId.uChar[0], label);
#endif
            }
            else
            {
                fprintf(log, "%d %s\n", featId.uId, label);
            }
        else
            fprintf(log, "%d\n", featId.uId);
        gr_label_destroy(reinterpret_cast<void*>(label));
        gr_uint16 numSettings = gr_fref_n_values(f);
        for (gr_uint16 j = 0; j < numSettings; j++)
        {
            gr_int16 value = gr_fref_value(f, j);
            label = reinterpret_cast<char *>(gr_fref_value_label
                (f, j, &langId, gr_utf8, &length));
            fprintf(log, "\t%d\t%s\n", value, label);
            gr_label_destroy(reinterpret_cast<void*>(label));
        }
    }
    gr_uint16 numLangs = gr_face_n_languages(face);
    fprintf(log, "Feature Languages:");
    for (gr_uint16 i = 0; i < numLangs; i++)
    {
        FeatID langID;
        langID.uId = gr_face_lang_by_index(face, i);
        langID.uId = swap32(langID.uId);
        fprintf(log, "\t");
        for (size_t j = 0; j < 4; j++)
        {
            if ((langID.uChar[j]) >= 0x20 && (langID.uChar[j] < 0x80))
                fprintf(log, "%c", langID.uChar[j]);
        }
    }
    fprintf(log, "\n");
}

gr_feature_val * Parameters::parseFeatures(const gr_face * face) const
{
    gr_feature_val * featureList = NULL;
    const char * pLang = NULL;
    FeatID lang;
    lang.uId = 0;
    if (features && (pLang = strstr(features, "lang=")))
    {
        pLang += 5;
        size_t i = 0;
        while ((i < 4) && (*pLang != '0') && (*pLang != '&'))
        {
            lang.uChar[i] = *pLang;
            ++pLang;
            ++i;
        }
        lang.uId = swap32(lang.uId);
    }
    featureList = gr_face_featureval_for_lang(face, lang.uId);
    if (!features || strlen(features) == 0)
        return featureList;
    size_t featureLength = strlen(features);
    const char * name = features;
    const char * valueText = NULL;
    size_t nameLength = 0;
    gr_int32 value = 0;
    FeatID featId;
    const gr_feature_ref* ref = NULL;
    featId.uId = 0;
    for (size_t i = 0; i < featureLength; i++)
    {
        switch (features[i])
        {
            case ',':
            case '&':
                value = atoi(valueText);
                if (ref)
                {
                    gr_fref_set_feature_value(ref, value, featureList);
                    ref = NULL;
                }
                valueText = NULL;
                name = features + i + 1;
                nameLength = 0;
                featId.uId = 0;
                break;
            case '=':
                if (nameLength <= 4)
                {
                    featId.uId = swap32(featId.uId);
                    ref = gr_face_find_fref(face, featId.uId);
                }
                if (!ref)
                {
                    featId.uId = atoi(name);
                    ref = gr_face_find_fref(face, featId.uId);
                }
                valueText = features + i + 1;
                name = NULL;
                break;
            default:
                if (valueText == NULL)
                {
                    if (nameLength < 4)
                    {
                        featId.uChar[nameLength++] = features[i];
                    }
                }
        }
        if (ref)
        {
            value = atoi(valueText);
            gr_fref_set_feature_value(ref, value, featureList);
            if (featId.uId > 0x20000000)
            {
                featId.uId = swap32(featId.uId);
                fprintf(log, "%c%c%c%c=%d\n", featId.uChar[0], featId.uChar[1],
                         featId.uChar[2], featId.uChar[3], value);
            }
            else
                fprintf(log, "%u=%d\n", featId.uId, value);
            ref = NULL;
        }
    }
    return featureList;
}

int Parameters::testFileFont() const
{
    int returnCode = 0;
//    try
    {
        // use the -trace option to specify a file
#ifndef DISABLE_TRACING
        graphite_start_logging(trace, static_cast<GrLogMask>(mask));
#endif

        gr_face *face = NULL;
        if (enableCache)
            face = gr_make_file_face_with_seg_cache(fileName, 1000, gr_face_preloadGlyphs | gr_face_dumbRendering);
        else
            face = gr_make_file_face(fileName, gr_face_preloadGlyphs);

        if (!face)
        {
            fprintf(stderr, "Invalid font, failed to read or parse tables\n");
            return 3;
        }
        if (charLength == 0)
        {
            printFeatures(face);
            gr_face_destroy(face);
            graphite_stop_logging();
            return 0;
        }

        gr_font *sizedFont = gr_make_font(pointSize * dpi / 72, face);
        gr_feature_val * featureList = NULL;
#if 0
        layout.setStartOfLine(parameters.lineStart);
        layout.setEndOfLine(parameters.lineEnd);
        layout.setDumbFallback(true);
        if (parameters.justification)
            layout.setJustifier(&justifier);
        else
            layout.setJustifier(NULL);
        layout.setRightToLeft(parameters.rtl);
        if (parameters.ws) layout.setTrailingWs(gr::ktwshAll);

        std::ofstream logStream("grSegmentLog.txt");
        assert(logStream.is_open());
        layout.setLoggingStream(&logStream);
        gr::Segment * pSegment = NULL;
        //try
        //{
          if (parameters.useLineFill)
          {
              pSegment = new gr::LineFillSegment(fileFont, &textSrc, &layout, 
                                                 0, parameters.charLength, 
                                                 parameters.width);
              printf("LineFillSegment line start=%d line end=%d\n", 
                     parameters.lineStart, parameters.lineEnd);
              if (parameters.justification && pSegment)
              {
                  printf("max shrink %f max stretch %f\n", pSegment->maxShrink(),
                      pSegment->maxShrink());
              }
          }
          else
          {
              pSegment = new gr::RangeSegment(fileFont, &textSrc, &layout, 
                                   0, parameters.charLength);
              if (pSegment)
                printf("RangeSegment line start=%d line end=%d rtl=%d\n", 
                       parameters.lineStart, parameters.lineEnd,
                       pSegment->rightToLeft());
          }
#endif
       Gr2TextSrc textSrc(pText32, charLength);
       {
        gr_segment* pSeg = NULL;
        if (features)
        {
            featureList = parseFeatures(face);
            pSeg = gr_make_seg(sizedFont,
                face, 0, featureList, textSrc.utfEncodingForm(),
                textSrc.get_utf_buffer_begin(), textSrc.getLength(), rtl);
        }
        else
        {
            pSeg = gr_make_seg(sizedFont, face, 0, NULL, textSrc.utfEncodingForm(),
                textSrc.get_utf_buffer_begin(), textSrc.getLength(), rtl);
        }
        int i = 0;
#ifndef NDEBUG
        int numSlots = gr_seg_n_slots(pSeg);
#endif
//        size_t *map = new size_t [seg.length() + 1];
        size_t *map = (size_t*)malloc((gr_seg_n_slots(pSeg) + 1) * sizeof(size_t));
        for (const gr_slot* slot = gr_seg_first_slot(pSeg); slot; slot = gr_slot_next_in_segment(slot), ++i)
        { map[i] = (size_t)slot; }
        map[i] = 0;
        fprintf(log, "pos  gid   attach\t     x\t     y\tins bw\t  chars\t\tUnicode\t");
        fprintf(log, "\n");
        i = 0;
        for (const gr_slot* slot = gr_seg_first_slot(pSeg); slot; slot = gr_slot_next_in_segment(slot), ++i)
        {
            // consistency check for last slot
            assert((i + 1 < numSlots) || (slot == gr_seg_last_slot(pSeg)));
            float orgX = gr_slot_origin_X(slot);
            float orgY = gr_slot_origin_Y(slot);
            fprintf(log, "%02d  %4d %3d@%d,%d\t%6.1f\t%6.1f\t%2d%4d\t%3d %3d\t",
                    i, gr_slot_gid(slot), lookup(map, (size_t)gr_slot_attached_to(slot)),
                    gr_slot_attr(slot, pSeg, gr_slatAttX, 0),
                    gr_slot_attr(slot, pSeg, gr_slatAttY, 0), orgX, orgY, gr_slot_can_insert_before(slot) ? 1 : 0,
                    gr_cinfo_break_weight(gr_seg_cinfo(pSeg, gr_slot_original(slot))), gr_slot_before(slot), gr_slot_after(slot));
           
            if (pText32 != NULL)
            {
                fprintf(log, "%7x\t%7x",
                    pText32[gr_slot_before(slot) + offset],
                    pText32[gr_slot_after(slot) + offset]);
            }
#if 0
            if (parameters.justification)
            {
                // only level 0 seems to be supported without an assertion
                for (int level = 0; level < 1; level++)
                {
                    printf("\t% 2d %6.1f %6.1f %6.1f %d", level,
                        info.maxShrink(level),
                        info.maxStretch(level), info.stretchStep(level),
                        info.justWeight(level));
                    if (info.maxShrink(level) == 0.0f &&
                        info.maxStretch(level) == 0.0f &&
                        info.stretchStep(level) == 0.0f &&
                        info.justWeight(level) == 0)
                        break;
                }
            }
            printf("\n");
            ++i;
            ++gi;
#endif
            fprintf(log, "\n");
        }
        assert(i == numSlots);
        // assign last point to specify advance of the whole array
        // position arrays must be one bigger than what countGlyphs() returned
        float advanceWidth = gr_seg_advance_X(pSeg);
        fprintf(log, "Advance width = %6.1f\n", advanceWidth);
        unsigned int numchar = gr_seg_n_cinfo(pSeg);
        gr_uint32 *firsts = (gr_uint32 *)malloc(numchar * sizeof(gr_uint32));
        gr_uint32 *lasts = (gr_uint32 *)malloc(numchar * sizeof(gr_uint32));
        gr_seg_char_slots(pSeg, firsts, lasts, 0, 0);
        fprintf(log, "\nChar\tUnicode\tBefore\tAfter\n");
        for (unsigned int j = 0; j < numchar; j++)
        {
            fprintf(log, "%d\t%04X\t%d\t%d\n", j, gr_cinfo_unicode_char(gr_seg_cinfo(pSeg, j)), firsts[j], lasts[j]);
        }
        free(map);
        gr_seg_destroy(pSeg);
       }
        if (featureList) gr_featureval_destroy(featureList);
        gr_font_destroy(sizedFont);
        gr_face_destroy(face);
//            delete featureParser;
        // setText copies the text, so it is no longer needed
//        delete [] parameters.pText32;
//        logStream.close();
    }
//    catch (...)
//    {
//        printf("Exception occurred\n");
//        returnCode = 5;
//    }
#ifndef DISABLE_TRACING
    if (trace) graphite_stop_logging();
#endif
    return returnCode;
}

int lookup(size_t *map, size_t val)
{
    int i = 0;
    for ( ; map[i] != val && map[i]; i++) {}
    return map[i] ? i : -1;
}

int main(int argc, char *argv[])
{
    
    Parameters parameters;
    
    if (!parameters.loadFromArgs(argc, argv))
    {
        fprintf(stderr,"Usage: %s [options] fontfile utf8text \n",argv[0]);
        fprintf(stderr,"Options: (default in brackets)\n");
        fprintf(stderr,"-dpi d\tDots per Inch (72)\n");
        fprintf(stderr,"-pt d\tPoint size (12)\n");
        fprintf(stderr,"-codes\tEnter text as hex code points instead of utf8 (false)\n");
        fprintf(stderr,"\te.g. %s font.ttf -codes 1000 102f\n",argv[0]);
        //fprintf(stderr,"-ls\tStart of line = true (false)\n");
        //fprintf(stderr,"-le\tEnd of line = true (false)\n");
        fprintf(stderr,"-rtl\tRight to left = true (false)\n");
        //fprintf(stderr,"-ws\tAllow trailing whitespace = true (false)\n");
        //fprintf(stderr,"-linefill w\tuse a LineFillSegment of width w (RangeSegment)\n");
        fprintf(stderr,"\nIf a font, but no text is specified, then a list of features will be shown.\n");
        fprintf(stderr,"-feat f=g\tSet feature f to value g. Separate multiple features with ,\n");
        fprintf(stderr,"-log out.log\tSet log file to use rather than stdout\n");
        fprintf(stderr,"-trace trace.xml\tDefine a file for the XML trace log\n");
        fprintf(stderr,"-mask mask\tDefine the mask to use for trace logging\n");
        fprintf(stderr,"-cache\tEnable Segment Cache\n");
        return 1;
    }
    return parameters.testFileFont();
}

