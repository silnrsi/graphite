#include "XmlTraceLogTags.h"







// start this at same line number as in XmlTraceLogTags.h
const XmlTraceLogTag xmlTraceLogElements[NumElements] = {
    XmlTraceLogTag("GraphitengLog", GRLOG_ALL),
    XmlTraceLogTag("Face", GRLOG_FACE),
    XmlTraceLogTag("Glyphs", GRLOG_FACE),
    XmlTraceLogTag("GlyphFace", GRLOG_FACE),
    XmlTraceLogTag("Attr", GRLOG_FACE),
    XmlTraceLogTag("Silf", GRLOG_FACE),
    XmlTraceLogTag("SilfSub", GRLOG_FACE),
    XmlTraceLogTag("Pass", GRLOG_FACE),
    XmlTraceLogTag("Segment", GRLOG_SEGMENT),

    XmlTraceLogTag("Error", GRLOG_ALL),
    XmlTraceLogTag("Warning", GRLOG_ALL)
};






// start this at same line number as in XmlTraceLogTags.h
const char * xmlTraceLogAttributes[NumAttributes] = {
    "version",
    "major",
    "minor",
    "num",
    "glyphId",
    "advance",
    "advanceX",
    "advanceY",
    "attrId",
    "attrVal",
    "compilerMajor",
    "compilerMinor",
    "numPasses",//AttrNumPasses,
    "subPass",//AttrSubPass,
    "posPass",//AttrPosPass,
    "justPass",//AttrJustPass,
    "bidiPass",//AttrBidiPass,
    "preContext",//AttrPreContext,
    "postContext",//AttrPostContext,
    "pseudoGlyph",//AttrPseudoGlyph,
    "breakWeight",//AttrBreakWeight,
    "directionality",//AttrDirectionality,
    "numJustLevels",//AttrNumJustLevels,
    "numLigCompAttr",//AttrLigComp,
    "numUserDefinedAttr",//AttrUserDefn,
    "maxNumLigComp",//AttrNumLigComp,
    "numCriticalFeatures",//AttrNumCritFeatures,
    "lineBreakglyph",//,AttrLBGlyph,
    "numPseudo",
    "passId",//AttrPassId,
    "flags",//AttrFlags,
    "maxRuleLoop",//AttrMaxRuleLoop,
    "maxRuleContext",//AttrMaxRuleContext,
    "maxBackup",//AttrMaxBackup,
    "numRules",//AttrNumRules,
    "numRows",//AttrNumRows,
    "numTransitionStates",//AttrNumTransition,
    "numSuccessStates",//AttrNumSuccess,
    "numColumns",//AttrNumColumns,
    "numRanges",//AttrNumRanges,
    "minPrecontext",//AttrMinPrecontext,
    "maxPrecontext"//AttrMaxPrecontext,
};
