/*  GRAPHITENG LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
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
    ElementSubSeg,

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
