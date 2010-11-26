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
#include <iconv.h>

#include "graphiteng/Types.h"
#include "graphiteng/GrSegment.h"
#include "graphiteng/Slot.h"
#include "graphiteng/GrFont.h"
#include "graphiteng/GrFace.h"
#include "graphiteng/XmlLog.h"

#include "graphiteng/CharInfo.h"

namespace gr2 = org::sil::graphite::v2;

#if !defined WORDS_BIGENDIAN || defined PC_OS
#define swap16(x) (((x & 0xff) << 8) | ((x & 0xff00) >> 8))
#define swap32(x) (((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x & 0xff000000) >> 24))
#else
#define swap16(x) (x)
#define swap32(x) (x)
#endif

class GrngTextSrc
{

public:
    GrngTextSrc(const gr2::uint32* base, size_t len) : m_buff(base), m_len(len) { }
    gr2::encform utfEncodingForm() const { return gr2::kutf32; }
    size_t getLength() const { return m_len; }
    const void* get_utf_buffer_begin() const { return m_buff; }

private:
    const gr2::uint32* m_buff;
    size_t m_len;
};

#ifndef HAVE_STRTOF
float strtof(char * text, char ** ignore)
{
  return static_cast<float>(atof(text));
}
#endif

#ifndef HAVE_STRTOL
long strtol(char * text, char ** ignore)
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
    gr2::Features* parseFeatures(const gr2::GrFace * face) const;
    void printFeatures(const gr2::GrFace * face) const;
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
    mask = gr2::GRLOG_ALL;
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
            charLength = convertUtf("utf8","utf32",pText, pText32);
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



#if 0
void listFeatures(gr::Font & font)
{
    std::pair< gr::FeatureIterator, gr::FeatureIterator > features = font.getFeatures();

    gr::FeatureIterator i = features.first;
    printf("%d Features\n", features.second - features.first);
    while (i != features.second)
    {
        grutils::FeatId fId;
        fId.num = *i;
        fId.label[4] = '\0';// null terminate
        bool isCharId = (fId.label[0] >= 0x20 && fId.label[0] < 0x80 &&
            (fId.label[1] == 0 || fId.label[1] >= 0x20) && fId.label[1] < 0x80 &&
            (fId.label[2] == 0 || fId.label[2] >= 0x20) && fId.label[2] < 0x80 &&
            (fId.label[3] == 0 || fId.label[3] >= 0x20) && fId.label[3] < 0x80);
        if (isCharId)
        {
            printf("%d\t%c%c%c%c\t",fId.num, fId.label[3], fId.label[2], fId.label[1], fId.label[0]);
        }
        else
            printf("%d\t\t", fId.num);
        utf16 featLabel[128];
        lgid enUS=0x0409;
        if (font.getFeatureLabel(i,enUS, featLabel))
        {
            utf8 * pFeatLabel = NULL;
            int n = convertUtf("utf16","utf8", featLabel, pFeatLabel);
            if (n)
                printf("%s", pFeatLabel);
        }

        printf("\n");
        std::pair< gr::FeatureSettingIterator, gr::FeatureSettingIterator > settings = 
           font.getFeatureSettings(i);
        gr::FeatureSettingIterator iS = settings.first;
        while(iS != settings.second)
        {
            printf("\t%d\t", *iS);
            if (font.getFeatureSettingLabel (iS, enUS, featLabel))
            {
                gr::utf8 * pSettingLabel = NULL;
                int n = convertUtf("utf16","utf8", featLabel, pSettingLabel);
                if (n)
                    printf("%s", pSettingLabel);
            }
            printf("\n");
            ++iS;
        }
        ++i;
    }
    std::pair<gr::LanguageIterator,gr::LanguageIterator> aSupported
         = font.getSupportedLanguages();
    printf("%d Language features:", aSupported.second - aSupported.first);
    gr::LanguageIterator iL = aSupported.first;
    while (iL != aSupported.second)
    {
        printf("\t%s", (*iL).rgch);
        ++iL;
    }
    printf("\n");
}
#endif

union FeatID
{
    gr2::uint8 uChar[4];
    gr2::uint32 uId;
};

void Parameters::printFeatures(const gr2::GrFace * face) const
{
    gr2::uint16 numFeatures = gr2::face_n_fref(face);
    fprintf(stdout, "%d features\n", numFeatures);
    gr2::uint16 langId = 0x0409;
    for (gr2::uint16 i = 0; i < numFeatures; i++)
    {
        gr2::FeatureRef * f = gr2::face_fref(face, i);
        gr2::uint32 length = 0;
        char * label = reinterpret_cast<char *>(gr2::feature_label(face, f, &langId, gr2::kutf8, &length));
        FeatID featId;
        featId.uId = gr2::fref_id(f);
        if (label)
            if ((featId.uChar[0] >= 0x20 && featId.uChar[0] < 0x7F) &&
                (featId.uChar[1] >= 0x20 && featId.uChar[1] < 0x7F) &&
                (featId.uChar[2] >= 0x20 && featId.uChar[2] < 0x7F) &&
                (featId.uChar[3] >= 0x20 && featId.uChar[3] < 0x7F))
            {
#ifdef WORDS_BIGENDIAN
                printf("%d %c%c%c%c %s\n", featId.uId, featId.uChar[0], featId.uChar[1],
                       featId.uChar[2], featId.uChar[3], label);
#else
                printf("%d %c%c%c%c %s\n", featId.uId, featId.uChar[3], featId.uChar[2],
                       featId.uChar[1], featId.uChar[0], label);
#endif
            }
            else
            {
                printf("%d %s\n", featId.uId, label);
            }
        else
            printf("%d\n", featId.uId);
        gr2::destroy_feature_label(reinterpret_cast<void*>(label));
        gr2::uint16 numSettings = gr2::num_feature_settings(f);
        for (gr2::uint16 j = 0; j < numSettings; j++)
        {
            gr2::int16 value = gr2::feature_setting_value(f, j);
            label = reinterpret_cast<char *>(gr2::feature_setting_label
                (face, f, j, &langId, gr2::kutf8, &length));
            printf("\t%d\t%s\n", value, label);
            gr2::destroy_feature_label(reinterpret_cast<void*>(label));
        }

        gr2::destroy_FeatureRef(f);
    }
    gr2::uint16 numLangs = gr2::face_n_languages(face);
    printf("Feature Languages:");
    for (gr2::uint16 i = 0; i < numFeatures; i++)
    {
        FeatID langID;
        langID.uId = gr2::face_lang_by_index(face, i);
        langID.uId = swap32(langID.uId);
        printf("\t");
        for (size_t i = 0; i < 4; i++)
        {
            if ((langID.uChar[i]) >= 0x20 && (langID.uChar[i] < 0x80))
                printf("%c", langID.uChar[i]);
        }
    }
    printf("\n");
}

gr2::Features * Parameters::parseFeatures(const gr2::GrFace * face) const
{
    gr2::Features * featureList = NULL;
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
    featureList = gr2::face_features_for_lang(face, lang.uId);
    if (!features || strlen(features) == 0)
        return featureList;
    size_t featureLength = strlen(features);
    const char * name = features;
    const char * valueText = NULL;
    size_t nameLength = 0;
    size_t valueLength = 0;
    gr2::int32 value = 0;
    FeatID featId;
    gr2::FeatureRef* ref = NULL;
    featId.uId = 0;
    for (size_t i = 0; i < featureLength; i++)
    {
        switch (features[i])
        {
            case '&':
                value = atoi(valueText);
                if (ref)
                {
                    gr2::fref_set_feature_value(ref, value, featureList);
                    gr2::destroy_FeatureRef(ref);
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
                    ref = gr2::face_find_fref(face, featId.uId);
                }
                if (!ref)
                {
                    featId.uId = atoi(name);
                    ref = gr2::face_find_fref(face, featId.uId);
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
            gr2::fref_set_feature_value(ref, value, featureList);
            gr2::destroy_FeatureRef(ref);
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
        //FILE * logFile = fopen("graphitengTrace.xml", "wb");
#ifndef DISABLE_TRACING
        gr2::graphite_start_logging(trace, static_cast<gr2::GrLogMask>(mask));
#endif

        gr2::GrFace *face = NULL;
        if (enableCache)
            face = gr2::make_file_face_with_seg_cache(fileName, gr2::ePreload, 1000);
        else
            face = gr2::make_file_face(fileName, gr2::ePreload);

        if (!face)
        {
            fprintf(stderr, "Invalid font, failed to read or parse tables\n");
            return 3;
        }
        if (charLength == 0)
        {
            printFeatures(face);
            gr2::destroy_face(face);
            gr2::graphite_stop_logging();
            return 0;
        }

        gr2::GrFont *sizedFont = gr2::make_font(pointSize * dpi / 72, face);
        gr2::Features * featureList = NULL;
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
       GrngTextSrc textSrc(pText32, charLength);
       {
        gr2::GrSegment* pSeg = NULL;
        if (features)
        {
            featureList = parseFeatures(face);
            pSeg = gr2::make_seg_using_features(sizedFont,
                face, 0, featureList, textSrc.utfEncodingForm(),
                textSrc.get_utf_buffer_begin(), textSrc.getLength(), rtl);
        }
        else
        {
            pSeg = gr2::make_seg(sizedFont, face, 0, textSrc.utfEncodingForm(),
                textSrc.get_utf_buffer_begin(), textSrc.getLength(), rtl);
        }
        int i = 0;
//        size_t *map = new size_t [seg.length() + 1];
        size_t *map = (size_t*)malloc((gr2::seg_n_slots(pSeg) + 1) * sizeof(size_t));
        for (const gr2::Slot* slot = gr2::seg_first_slot(pSeg); slot; slot = gr2::slot_next_in_segment(slot), ++i)
        { map[i] = (size_t)slot; }
        map[i] = 0;
        fprintf(log, "pos  gid   attach\t     x\t     y\tins bw\t  chars\t\tUnicode\t");
        fprintf(log, "\n");
        i = 0;
        for (const gr2::Slot* slot = seg_first_slot(pSeg); slot; slot = slot_next_in_segment(slot), ++i)
        {
            float orgX = slot_origin_X(slot);
            float orgY = slot_origin_Y(slot);
            fprintf(log, "%02d  %4d %3d@%d,%d\t%6.1f\t%6.1f\t%2d%4d\t%3d %3d\t",
                    i, slot_gid(slot), lookup(map, (size_t)slot_attached_to(slot)),
                    slot_attr(slot, pSeg, gr2::kslatAttX, 0),
                    slot_attr(slot, pSeg, gr2::kslatAttY, 0), orgX, orgY, slot_is_insert_before(slot) ? 1 : 0,
                    gr2::cinfo_break_weight(seg_cinfo(pSeg, slot_original(slot))), slot_before(slot), slot_after(slot));
           
            if (pText32 != NULL)
            {
                fprintf(log, "%7x\t%7x",
                    pText32[slot_before(slot) + offset],
                    pText32[slot_after(slot) + offset]);
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
        // assign last point to specify advance of the whole array
        // position arrays must be one bigger than what countGlyphs() returned
        float advanceWidth = seg_advance_X(pSeg);
        fprintf(log, "Advance width = %6.1f\n", advanceWidth);
        free(map);
        gr2::destroy_seg(pSeg);
       }
        if (featureList) gr2::features_destroy(featureList);
        gr2::destroy_font(sizedFont);
        gr2::destroy_face(face);
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
    if (trace) gr2::graphite_stop_logging();
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
        fprintf(stderr,"-ls\tStart of line = true (false)\n");
        fprintf(stderr,"-le\tEnd of line = true (false)\n");
        fprintf(stderr,"-rtl\tRight to left = true (false)\n");
        fprintf(stderr,"-ws\tAllow trailing whitespace = true (false)\n");
        //fprintf(stderr,"-linefill w\tuse a LineFillSegment of width w (RangeSegment)\n");
        fprintf(stderr,"\nIf a font, but no text is specified, then a list of features will be shown.\n");
        fprintf(stderr,"-feat f=g\tSet feature f to value g. Separate multiple features with &\n");
        fprintf(stderr,"-log out.log\tSet log file to use rather than stdout\n");
        fprintf(stderr,"-trace trace.xml\tDefine a file for the XML trace log\n");
        fprintf(stderr,"\nTrace Logs are written to grSegmentLog.txt if graphite was compiled with\n--enable-tracing.\n");
        fprintf(stderr,"-cache\tEnable Segment Cache\n");
        return 1;
    }
    return parameters.testFileFont();
}

