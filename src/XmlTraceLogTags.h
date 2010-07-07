#pragma once

#include <graphiteng/Types.h>
#include <graphiteng/XmlLog.h>

#ifdef ENABLE_DEEP_TRACING
#undef DISABLE_TRACING
#endif

#ifndef DISABLE_TRACING

namespace org { namespace sil { namespace graphite { namespace v2 {

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
    ElementPseudo,
    ElementClassMap,
    ElementLookupClass,
    ElementLookup,
    ElementRange,
    ElementRuleMap,
    ElementRule,
    ElementStartState,
    ElementStateTransitions,
    ElementRow,
    ElementData,
    ElementConstraint,
    ElementConstraints,
    ElementActions,
    ElementAction,
    ElementFeatures,
    ElementFeature,
    ElementFeatureSetting,
    ElementSegment,
    ElementSlot,
    ElementText,
    ElementOpCode,
    ElementTestRule,
    ElementDoRule,
    ElementRunPass,
    ElementParams,
    ElementPush,

    ElementError,
    ElementWarning,
    NumElements // Last
} XmlTraceLogElement;



// start this at same line number as in XmlTraceLogTags.cpp
typedef enum {
    AttrIndex,
    AttrVersion,
    AttrMajor,
    AttrMinor,
    AttrNum,
    AttrGlyphId,
    AttrAdvance,
    AttrAdvanceX,
    AttrAdvanceY,
    AttrAttrId,
    AttrAttrVal,
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
    AttrNumScripts,
    AttrLBGlyph,
    AttrNumPseudo,
    AttrNumClasses,
    AttrNumLinear,
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
    AttrFirstId,
    AttrLastId,
    AttrColId,
    AttrSuccessId,
    AttrRuleId,
    AttrContextLen,
    AttrState,
    AttrValue,
    AttrSortKey,
    AttrPrecontext,
    AttrAction,
    AttrActionCode,
    Attr0,
    Attr1,
    Attr2,
    Attr3,
    Attr4,
    Attr5,
    Attr6,
    Attr7,
    AttrLabel,
    AttrLength,
    AttrX,
    AttrY,
    AttrBefore,
    AttrAfter,
    AttrEncoding,
    AttrName,
    AttrResult,
    AttrDefault,

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

}}}} // namespace

#endif
