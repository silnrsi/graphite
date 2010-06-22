#ifndef XmlTraceLogTags_h
#define XmlTraceLogTags_h

#include <graphiteng/Types.h>
#include <graphiteng/XmlLog.h>



// start this at same line number as in XmlTraceLogTags.cpp
typedef enum {
    ElementTopLevel,
    ElementFace,
    ElementGlyphs,
    ElementGlyphFace,
    ElementAttr,
    ElementSilf,
    ElementSilfSub,
    ElementPass,
    ElementSegment,

    ElementError,
    ElementWarning,
    NumElements // Last
} XmlTraceLogElement;





// start this at same line number as in XmlTraceLogTags.cpp
typedef enum {
    AttrVersion,
    AttrMajor,
    AttrMinor,
    AttrNum,
    AttrGlyphId,
    AttrAdvance,
    AttrAdvanceX,
    AttrAdvanceY,
    AttrAttrId,
    AttrCompilerMajor,
    AttrCompilerMinor,
    AttrNumPasses,
    AttrSubPass,
    AttrPosPass,
    AttrJustPass,
    AttrBidiPass,
    AttrPreContext,
    AttrPostContext,
    AttrPseudoGlyph,
    AttrBreakWeight,
    AttrDirectionality,
    AttrNumJustLevels,
    AttrLigComp,
    AttrUserDefn,
    AttrNumLigComp,
    AttrNumCritFeatures,
    AttrLBGlyph,
    AttrNumPseudo,
    AttrPassId,
    AttrFlags,
    AttrMaxRuleLoop,
    AttrMaxRuleContext,
    AttrMaxBackup,
    AttrNumRules,
    AttrNumRows,
    AttrNumTransition,
    AttrNumSuccess,
    AttrNumColumns,
    AttrNumRanges,
    AttrMinPrecontext,
    AttrMaxPrecontext,
    NumAttributes // Last
} XmlTraceLogAttribute;

struct XmlTraceLogTag
{
public:
    XmlTraceLogTag(const char * name, uint32 flags) : mName(name), mFlags(flags) {}
    const char * mName;
    uint32 mFlags;
};

extern const XmlTraceLogTag xmlTraceLogElements[NumElements];
extern const char * xmlTraceLogAttributes[NumAttributes];

#endif
