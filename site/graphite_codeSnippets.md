---
layout: default
title: GDL Code Snippets
nav_order: 50
parent: Developers
---

# GDL Code Snippets

* [Diacritic attachment](graphite_codeSnippets#diacritic-attachment)
    * [Diacritics with calculated attachment points](graphite_codeSnippets#diacritics-with-calculated-attachment-points)
    * [Stacking diacritics](graphite_codeSnippets#stacking-diacritics)
    * [Bridging diacritics](graphite_codeSnippets#bridging-diacritics)
* [Ligatures (conjuncts)](graphite_codeSnippets#ligatures-conjuncts)
    * [Ligatures with components](graphite_codeSnippets#ligatures-with-components)
* [Contextual shaping](graphite_codeSnippets#contextual-shaping)
    * [Word-contextual shaping](graphite_codeSnippets#word-contextual-shaping)
* [Reordering](graphite_codeSnippets#reordering)
* [Splitting](graphite_codeSnippets#splitting)
* [Font features (variations)](graphite_codeSnippets#font-features-variations)
    * [Boolean feature](graphite_codeSnippets#boolean-feature)
    * [Multi-valued feature](graphite_codeSnippets#multi-valued-feature)

These snippets are intended as a quick reference. For complete documentation of the GDL language, download the reference paper:

[Download GDL.pdf](GDL.pdf){: .btn .btn-blue }

## Diacritic attachment

```
table(glyph)

// These can also be classes, in which case the attachment point(s) 
// must be defined on each element of the class:
base = glyphid(34) {TopS = point(130m, 512m); BottomS = point(130m, 0m)};
upperdiac = glyphid(101) {TopM = point(86m, -12m)}
lowerdiac = glyphid(102) {BottomM = point(86m, 43m)};

endtable; // glyph

table(positioning)

base  upperdiac {attach {to=@1; at=TopS; with=TopM}};
base  lowerdiac {attach {to=@1; at=BottomS; with=BottomM}};

endtable; // positioning
```

### Diacritics with calculated attachment points

```
table(glyph)

base = glyphid(...)
    {TopS = point(bb.lsb + bb.width/2, bb.top); 
     BottomS = point(bb.lsb + bb.width/2, bb.bottom)};
upperdiac = glyphid(...) {TopM = point(bb.width/2, bb.bottom - 10m)}
lowerdiac = glyphid(...) {BottomM = point(bb.width/2, bb.top + 10m)};

endtable; // glyph

table(positioning)

base  upperdiac {attach {to=@1; at=TopS; with=TopM}};
base  lowerdiac {attach {to=@1; at=BottomS; with=BottomM}};

endtable; // positioning
```

### Stacking diacritics

The following example shows how to handle a stack/chain of both upper and lower diacritic attachments. Note that, as specified by the Unicode standard, lower diacritics are first.

```
table(glyph)

base1 = glyphid(34) {TopS = point(130m, 512m); BottomS = point(130m, 0m)};
base1 = glyphid(35) {TopS = point(130m, 5108m); BottomS = point(130m, -18m)};

upperdiac1 = glyphid(101) {TopM = point(86m, -12m); TopS = point(86m, 43m)};
upperdiac2 = glyphid(105) {TopM = point(81m, -10m); TopS = point(81m, 52m)};

lowerdiac1 = glyphid(102) {BottomM = point(79m, 43m); BottomS = point(79m, -3m)};
lowerdiac2 = glyphid(103) {BottomM = point(77m, 48m); BottomS = point(77m, -5m)};
// etc.

base = (base1, base2, ...);
upperdiac = (upperdiac1, upperdiac2, ...);
lowerdiac = (lowerdiac1, lowerdiac2, ...);
    
takes_lower = (base, lowerdiac); // lower diacritics can be attached to bases or other lower diacs
takes_upper = (base, upperdiac); // upper diacritics can be attached to bases or other upper diacs

endtable; // glyph

table(positioning)

// Optionally permit up to three intervening lower diacritics:
#define LOWERSEQ [ lowerdiac [ lowerdiac  lowerdiac? ]? ]?

takes_lower  lowerdiac {attach {to=@1; at=BottomS; with=BottomM}} / _ ^ _;

takes_upper  upperdiac {attach {to=@1; at=TopS; with=TopM}} / _ LOWERSEQ  ^ _;

endtable; // positioning
```

### Bridging diacritics

Bridging diacritics must be centered over the two bases they bridge and must be higher than the tallest.

```
table(glyph)

base1 = ...;
base2 = ...;
bridgediac = ...;

endtable; // glyph

table(positioning)

bridgediac {shift.y = max(@B1.boundingbox.top, @B2.boundingbox.top) - @D.boundingbox.bottom + 100m ; // gap of 100m
            shift.x = (@B2.boundingbox.width - @B1.boundingbox.width)/2}
    / base1=B1  _=D  base2=B2;

endtable; // positioning
If the base glyphs have other diacritics attached, using @B1.boundingbox.top.1 will give the bounding box of the base plus diacritics.
```

## Ligatures (conjuncts)

Ligatures or conjuncts occur when multiple characters combine to form a single glyph shape.

```
table(glyph)

char1 = ...;
char2 = ...;
ligature = ...;

endtable; // glyph

table(substitution)

char1  char2  >  ligature:(1 2)  _ ;

endtable; // substitution
```

### Ligatures with components

Ligature components correspond to sub-regions of the ligature glyph that visually correspond to the original characters. Selecting and manipulating the visual components allows manipulating the underlying characters.

{: .blue-note }
> **Note**
>
> This feature is only supported by the original Graphite engine; it is not supported by Graphite2.

```
table(glyph)

char1 = ...;
char2 = ...;

ligature = ... // <-- specify which glyph
    // Define the visual components of the conjunct.
    // This conjunct has three components:
    // the top-left, the bottom-left, and the right side.
    { component {cTL= box(0, bb.height/2,   aw/2, bb.top);
                 cBL= box(0, bb.bottom,     aw/2, bb.height/2);
                 cR = box(aw/2, bb.bottom,  aw, bb.top) } };

endtable; // glyph

table(substitution)

char1  char2  char3  >  ligature:(1 2 3) {component {cTL.ref=@1; cBL.ref=@2; cR=@3}}  _  _;

endtable; // substitution
```

## Contextual shaping

Many scripts have characters that must use alternate forms depending on neighboring letters.

```
table(glyph)

// Define the glyphs and classes:
matraI = ...;
matraI_wide = ...;
matraI_wider = ...;
matraI_widest = ...;

class_wide = ...;
class_wider = ...;
class_widest = ...;

endtable; // glyph

table(substitution)

// Choose the width of the matra-I that matches the width of the neighboring letter:
matraI  class_wide     >  matraI_wide  @2;
matraI  class_wider    >  matraI_wider  @2;
matraI  class_widest   >  matraI_widest  @2;

endtable;
```

### Word-contextual shaping

Arabic is an example of a script whose characters take on alternate forms depending on their location within the word: initial, medial, final, isolate.

```
table(glyph)

// All characters are initially isolate forms.

// Define the individual glyphs:
beh = ...;
behInit = ...;
// etc.

// Elements of these four classes must correspond:
class_isolate = (beh,     teh,     theh,     peh,     teheh,     ...);
class_initial = (behInit, tehInit, thehInit, pehInit, tehehInit, ...);
class_medial  = (behMed,  behMed,  thehMed,  pehMed,  tehehMed,  ...);
class_final   = (behFin,  behFin,  thehFin,  pehFin,  tehehFin,  ...);

endtable; // glyph

table(substitution)

// For any contiguous pair of word-forming letters:
//   turn the first from isolate to initial, or from final to medial
//   turn the second to final
//   back up and treat the second as the first of the next pair to consider
// Any unprocessed letters are left as isolates.

(class_isolate class_final)  (class_isolate)  >  (class_initial class_medial) (class_final)  /  _ ^ _;

endtable; // substitution
```

## Reordering

Many scripts of south and southeast Asia have characters that are rendered in an order that is different from their order in the data.

```
table(glyph)

// Define the glyphs or classes:
consonants = ...;
vowelLeftSide = ...;

endtable; // glyph

table(substitution)

consonant  vowelLeftSide  >  @2  @1;

endtable; // substitution
```

## Splitting

Many scripts of south and southeast Asia have characters that are rendered by two or more non-contiguous glyphs.

```
table(glyph)

// Define the glyphs or classes:
consonants = ...;

// If these are classes, the elements of the classes must correspond:
vowelSplit     = (vowel1,      vowel2,      vowel3, ...);
vowelLeftHalf  = (vowel1Left,  vowel2Left,  vowel3Left, ...);
vowelRightHalf = (vowel1Right, vowel2Right, vowel3Right, ...);

endtable; // glyph

table(substitution)

_  consonant  vowelSplit  >  vowelLeftHalf$3:3  @2  vowelRightHalf:3;

end-table; // substitution
```

## Font features (variations)

Font features provide a way to define alterative renderings that can be selected by the user.

### Boolean feature

```
table(feature)

altk {
    id = "altk";  // can be integer or 4-char string
    name.1033 = string("Alternate K");  // 1033 = LG_USENG, US English
    default = 0;
    settings {
        off {
            value = 0;
            name.1033 = string("False");
        }
        on {
            value = 1;
            name.1033 = string("True");
        }
    }
};

endtable; // feature

table(glyph)

g_k = ...;
g_k_alt = ...;

endtable; // glyph

table(substitution)

if (altk)
g_k  >  g_k_alt
endif;

endtable; // substitution
```

### Multi-valued feature

```
table(feature)

altk {
    id = "altk"; // can be integer or 4-char string
    name.1033 = string("Alternate K"); // 1033 = LG_USENG, US English
    default = 0;
    settings {
        form1 {
            value = 0;
            name.1033 = string("Form 1");
        }
        form2 {
            value = 1;
            name.1033 = string("Form 2");
        }
        form3 {
            value = 2;
            name.1033 = string("Form 3");
        }
        form4 {
            value = 3;
            name.1033 = string("Form 4");
        }
    }
};

endtable; // feature

table(glyph)

g_k = ...;
g_k1 = ...;
g_k2 = ...;
g_k3 = ...;
g_k4 = ...;

endtable; // glyph

table(substitution)

// Note that for mutually exclusive conditions, successive
// 'if' statements are more efficient than 'elseif'.

if (altk == form1)
g_k  >  g_k1;
endif;

if (altk == form2)
g_k  >  g_k2;
endif;

if (altk == form3)
g_k  >  g_k3;
endif;

if (altk == form4)
g_k  >  g_k3;
endif;

endtable; // substitution
```
