---
layout: default
title: Solutions to Tutorial Exercises
nav_order: 999
parent: Graphite Tutorial
grand_parent: Developers
---

# Solutions to tutorial exercises

## Exercise 2

```
#include "stddef.gdh"

table(sub)
  unicode(0x0024) > unicode(0x00A3);  // or U+0024 > U+00A3
endtable;
```

## Exercise 3a

```
#include "stddef.gdh"

table(glyph)
  gDollar = unicode(0x0024);
  gPound = unicode(0x00A3);
endtable;

table(sub)
  gDollar > gPound;
endtable;
```

## Exercise 3b

```
#include "stddef.gdh"

table(glyph)
  clsDigit = (U+0030..U+0039);
  gAsterisk = U+002A;
endtable;

table(sub)
  clsDigit > gAsterisk;
endtable;
```

## Exercise 4a

```
#include "stddef.gdh"

table(glyph)
  clsLower = unicode(0x61..0x7A);
  clsUpper = unicode(0x41..0x5A);
endtable;

table(sub)
  clsLower > clsUpper;
endtable;
```

## Exercise 4b

```
#include "stddef.gdh"

table(glyph)
  clsLowerVowel = (U+0061, U+0065, U+0069, U+006F, U+0075);  // a e i o u
  clsUpperVowel = (U+0041, U+0045, U+0049, U+004F, U+0055);  // A E I O U

  clsLowerCons = ((U+0062..U+0064), // b..d
    (U+0066..U+0068),     // f..h
    (U+006A..U+006E),     // j..n
    (U+0070..U+0074),     // p..t
    (U+0076..U+007A));    // v..z
  clsUpperCons = ((U+0042..U+0044), // B..D
    (U+0046..U+0048),     // F..H
    (U+004A..U+004E),     // J..N
    (U+0050..U+0054),     // P..T
    (U+0056..U+005A));    // V..Z
endtable;

table(sub)
  clsLowerVowel > clsUpperVowel;
  clsUpperCons > clsLowerCons;
endtable;
```

## Exercise 4c

```
#include "stddef.gdh"

table(glyph)
  clsRoman = ((U+0061..U+0069),  // a..i
    (U+006B..U+0075),    // k..u
    (U+0077..U+007a));   // w..z

  clsGreek = (U+03b1, // alpha
    U+03b2,   // beta
    U+03c7,   // chi
    U+03b4,   // delta
    U+03b5,   // epsilon
    U+03c6,   // phi
    U+03b3,   // gamma
    U+03b7,   // eta
    U+03b9,   // iota
    U+03ba,   // kappa
    U+03BB,   // lambda - use BB to avoid compiler bug
    U+03bc,   // mu
    U+03bd,   // nu
    U+03bf,   // omicron
    U+03d0,   // pi
    U+03b8,   // theta
    U+03c1,   // rho
    U+03c2,   // sigma
    U+03c4,   // tau
    U+03c5,   // upsilon
    U+03c9,   // omega
    U+03be,   // xi
    U+03c8,   // psi
    U+03b6);  // zeta
endtable;

table(sub)
  clsRoman > clsGreek;
endtable;
```

## Exercise 5a

Note that this program will generate warnings for the deletion rules.

{: .red-warning }
> **NOTE**
>
> Linux users have reported compilation errors for this exercise.

```
#include "stddef.gdh"

table(glyph)

  // Roman

  clsRoman = ((U+0061..U+0069),  // a..i
    (U+006B..U+0075),   // k..u
    (U+0077..U+007a));  // w..z

  gJ = U+006a;
  gV = U+0076;
  gH = U+0068;
  gT = U+0074;
  gP = U+0070;
  gS = U+0073;

  // Greek

  clsGreek = (U+03b1, // alpha
    U+03b2,   // beta
    U+03c7,   // chi
    U+03b4,   // delta
    U+03b5,   // epsilon
    U+03c6,   // phi
    U+03b3,   // gamma
    U+03b7,   // eta
    U+03b9,   // iota
    U+03ba,   // kappa
    U+03BB,   // lambda - use BB to avoid compiler bug
    U+03bc,   // mu
    U+03bd,   // nu
    U+03bf,   // omicron
    U+03d0,   // pi
    U+03b8,   // theta
    U+03c1,   // rho
    U+03c2,   // sigma
    U+03c4,   // tau
    U+03c5,   // upsilon
    U+03c9,   // omega
    U+03be,   // xi
    U+03c8,   // psi
    U+03b6);  // zeta

  gTheta = U+03b8;
  gPhi = U+03c6;
  gPsi = U+03c8;

endtable;

table(sub)
  // Delete j and v. Here we do not associate them
  // with any surface glyph, so it will be impossible
  // to select or manipulate them.
  gJ  >  _;
  gV  >  _;

  // Handle sequences.
  gT gH  >  gTheta:(1 2) _;
  gP gH  >  gPhi:(1 2) _;
  gP gS  >  gPsi:(1 2) _;

  // Everything else:
  clsRoman > clsGreek;
endtable;
```

## Exercise 5b

```
#include "stddef.gdh"

table(glyph)
  clsQ = (unicode(0x71), unicode(0x51));
  gU = unicode(0x75);
endtable;

table(sub)
  clsQ  _  >  clsQ  gU:1;

  // Note: you can also use the @ syntax to copy an item through unchanged:
  // clsQ  _  >  @1  gU:1;
endtable;
```

## Exercise 6a

```
#include "stddef.gdh"

table(glyph)
  gC = (U+0063, U+0043);
  gK = (U+006b, U+004b);
  gS = (U+0073, U+0053);
  clsSoftVowel =
    (U+0065, U+0069, U+0079,   // eiy
     U+0045, U+0049, U+0059);  // EIY
endtable;

table(sub)
  gC  >  gS  /  _  clsSoftVowel;
  gC  >  gK;
endtable;
```

## Exercise 6b

```
#include "stddef.gdh"

table(glyph)
  gQLower = unicode(0x71);
  gQUpper = unicode(0x51);
  gULower = unicode(0x75) ;
  gUUpper = unicode(0x55);

  clsUpper = unicode(0x41..0x5a);
endtable;

table(sub)
  _  >  gUUpper:1  /  gQUpper  _  clsUpper;
  _  >  gULower:1  /  (gQUpper gQLower)  _;
endtable;
```

## Exercise 6c

```
#include "stddef.gdh"

table(glyph)

  // Roman

  clsRoman = ((U+0061..U+0069),  // a..i
    (U+006B..U+0075),   // k..u
    (U+0077..U+007a));  // w..z

  gJ = U+006a;
  gV = U+0076;
  gH = U+0068;
  gT = U+0074;
  gP = U+0070;
  gS = U+0073;

  // Greek

  clsGreek = (U+03b1, // alpha
    U+03b2,   // beta
    U+03c7,   // chi
    U+03b4,   // delta
    U+03b5,   // epsilon
    U+03c6,   // phi
    U+03b3,   // gamma
    U+03b7,   // eta
    U+03b9,   // iota
    U+03ba,   // kappa
    U+03BB,   // lambda - use BB to avoid compiler bug
    U+03bc,   // mu
    U+03bd,   // nu
    U+03bf,   // omicron
    U+03d0,   // pi
    U+03b8,   // theta
    U+03c1,   // rho
    U+03c2,   // sigma
    U+03c4,   // tau
    U+03c5,   // upsilon
    U+03c9,   // omega
    U+03be,   // xi
    U+03c8,   // psi
    U+03b6);  // zeta

  gTheta = U+03b8;
  gPhi = U+03c6;
  gPsi = U+03c8;

  gSigma = U+03c3;
  gSigmaFinal = U+03c2;

endtable;

table(sub)
  // Delete j and v. Here we do not associate them
  // with any surface glyph, so it will be impossible
  // to select or manipulate them.
  gJ  >  _;
  gV  >  _;

  // Replace s with the right kind of sigma.
  gS  >  gSigma / _ clsRoman;  // ie, a Roman letter follows
  gS  >  gSigmaFinal;    // followed by space, punctuation, or nothing at all


  // Handle sequences.
  gT gH  >  gTheta:(1 2) _;
  gP gH  >  gPhi:(1 2) _;
  gP gS  >  gPsi:(1 2) _;

  // Everything else:
  clsRoman > clsGreek;

endtable;
```

## Exercise 7

```
#include "stddef.gdh"

// The following is true by default, so does not really need to 
// be stated. It needs to be true because we are overriding
// the value of followsHardC for soft vowels in the glyph table.
environment { AttributeOverride = true };

table(glyph)
  gC = (U+0063 U+0043);
  gK = (U+006b U+004b);
  gS = (U+0073 U+0053);
  clsLetter = ((U+0061..U+007a), (U+0041..U+005a))
            {followsHardC = true};
  clsSoftVowel =
     (U+0065, U+0069, U+0079,  // eiy
      U+0045, U+0049, U+0059)  // EIY
            {followsHardC = false}; // overrides above
endtable;

table(sub)
  gC  >  gK  /  _  clsLetter { followsHardC };
  gC  >  gS  /  _  clsLetter { !followsHardC };
endtable;

endenvironment;
```

## Exercise 9

```
#include "stddef.gdh"

#define hardC user1

table(glyph)
  gC = (U+0063 U+0043);
  gK = (U+006b U+004b);
  gS = (U+0073 U+0053);

  clsHardLetter = (
    (U+0061..U+0064), // a..d
    (U+0066..U+0068), // f..h
    (U+006A..U+0078), // j..x
     U+007A,          // z
    (U+0041..U+0044), // A..D
    (U+0046..U+0058), // F..H
    (U+004A..U+0058), // J..X
     U+005A);         // Z
  clsSoftLetter =
    (U+0065, U+0069, U+0079,  // eiy
     U+0045, U+0049, U+0059)  // EIY

endtable;

table(sub)

pass(1)
  // Mark the C based on its context.
  gC {hardC = true}  /  _  clsHardLetter;
  gC {hardC = false}  /  _  clsSoftLetter;
endpass;

pass(2);
  // Make the substitution.
  gC  >  gK  /  _ {hardC};
  gC  >  gS  /  _ {!hardC};
endpass;

endtable;
```

## Exercise 10a

```
#include "stddef.gdh"

table(glyph)
  clsDigit = (U+0030..U+0039);
endtable;

table(pos) { MUnits = 1000 }
  clsDigit {shift.y = -300m} ;
endtable;
```

## Exercise 10b

```
#include "stddef.gdh"

table(glyph)
  clsAUpper = U+0041;
  clsVWUpper = (U+0056, U+0057);
endtable;

table(pos)
  clsAUpper { kern.x = -175m } / clsVWUpper _ ;
  clsVWUpper { kern.x = -175m } / clsAUpper _ ;
endtable;
```

## Exercise 11

```
#include "stddef.gdh"

table(glyph)
  clsDotted = (unicode(0x69), unicode(0x6a)); // i, j
  clsDotless = (glyphid(194), glyphid(195));
  clsDiac = (unicode(0x0300..0x0304), unicode(0x0308), unicode(0x030c))
      { halfWidth = bb.width/2 };
  clsBase = (unicode(0x61..0x7a), unicode(0x41..0x5a), clsDotless)
      { halfWidth = bb.width/2;
        baseHt = bb.top + 100m;
        rtSide = rsb;
      };
endtable;

table(sub)
  // Remove any dot.
  clsDotted  >  clsDotless  /  _  clsDiac;
endtable;

table(pos)
  clsBase  clsDiac {
      // "0 – lsb - ... " is a workaround for a bug in the compiler.
      shift {
        x = 0 - lsb - halfWidth - @1.halfWidth - @1.rtSide;
        y = @1.baseHt - bb.bottom } };
endtable;
```

## Exercise 12a

```
#include "stddef.gdh"

environment { PointRadius = 0 } // workaround for hinting bug

table(glyph)
  clsDotted = (unicode(0x69), unicode(0x6a)); // i, j
  clsDotless = (glyphid(194), glyphid(195));
  clsDiac = (unicode(0x0300..0x0304), unicode(0x0308), unicode(0x030c))
      { upperAttPtM= point(lsb + bb.width/2, bb.bottom – 50m) };
  clsBase = (unicode(0x61..0x7a), unicode(0x41..0x5a), clsDotless)
      // An alternative to "advancewidth/2" is "lsb + bb.width/2".
      { upperAttPtS = point(advancewidth/2, bb.top) };
endtable;

table(sub)
  // Remove any dot.
  clsDotted  >  clsDotless  /  _ clsDiac;
endtable;

table(pos)
  clsBase  clsDiac { attach { to=@1; at=upperAttPtS; with=upperAttPtM } };
endtable;

endenvironment;
The following is the result when using Graide to create attachment points:

#include "stddef.gdh"

table(glyph)
  clsDotted = (unicode(0x69), unicode(0x6a)); // i, j
  clsDotless = (glyphid(194), glyphid(195));
endtable;

table(sub)
  // Remove any dot.
  clsDotted  >  clsDotless  /  _ cupperDia;
endtable;

// upperS and upperM are defined in the auto-generated file
table(pos)
  cTakesupperDia  cupperDia { attach { to=@1; at=upperS; with=upperM } };
endtable;
```

## Exercise 12b

```
#include "stddef.gdh"

environment { PointRadius = 0 } // workaround for hinting bug

table(glyph)
  clsDiac = (unicode(0x0316) unicode(0x0317), unicode(0x031f), unicode(0x0327..0x0330))
        { lowerM = point(lsb + bb.width/2, bb.top + 50m) };
  clsBase = (unicode(0x61..0x7a), unicode(0x41..0x5a))
        // An alternative to "advancewidth/2" is lsb + bb.width/2.
        { lowerS = point(advancewidth/2, bb.bottom) };
endtable;

table(pos)
  clsBase  clsDiac { attach { to=@1; at=lowerS; with=lowerM } };
endtable;

endenvironment;
The following is the result when using Graide to create attachment points:

#include "stddef.gdh"

// lowerS and lowerM are defined in the auto-generated file
table(pos)
  cTakeslowerDia  clowerDia { attach { to=@1; at=lowerS; with=lowerM } };
endtable;
```

## Exercise 12c

```
#include "stddef.gdh"

table(glyph)
  clsBase = (unicode(0x03b1) unicode(0x03b7), unicode(0x03c9))
        { attPtIota = point(lsb + bb.width/2, bb.bottom);
          attPtBreath = point(lsb + bb.width/2, bb.top)
    };
  // Adjust x-position of attachment point for eta. We are in 
  // effect overriding the value set above.
  gEta = unicode(0x03b7)
        { attPtIota = point(lsb + (bb.width*3/8), bb.bottom + 160m) };
  gIotaDiac = unicode(0x0345)
        { attPt = point(lsb + bb.width/2, bb.bottom + 50m) };
  clsBreathDiac = unicode(0x0313..0x0314)
        { attPt = point(lsb + bb.width/2, bb.bottom – 50m) };
endtable;

table(pos)
  clsBase
      clsBreathDiac { attach { to=@1; at=attPtBreath; with=attPt } }
      gIotaDiac { attach { to=@1; at=attPtIota; with=attPt } };

  clsBase  gIotaDiac { attach { to=@1; at= attPtIota; with=attPt } };
endtable;
```

The following is the result when using Graide to create attachment points:

```
#include "stddef.gdh"

// lowerIotaS and lowerIotaM are defined in the auto-generated file
table(pos)

  cTakeslowerIotaDia  clowerIotaDia {attach {to = @1; at = lowerIotaS; with = lowerIotaM}};

endtable;
```

## Exercise 13a

```
include "stddef.gdh"

table(glyph)
  clsDigit = (U+0030..U+0039);
endtable;

table(feature)

supersub {
  id = "digt";
  name.LG_USENG = string("Superscript or Subscript");
  default = super;
  settings {
    no {
      value = 0;
      name.LG_USENG=string("Neither")
    }
    super {
      value = 1; 
      name.LG_USENG=string("Superscript")
    }
    subsc { // unfortunately, can't call it "sub"
      value = 2;
      name.LG_USENG=string("Subscript")
    }
  }
}

endtable;

table(pos)

if (supersub == super)
  clsDigit {shift.y = 300m};
elseif (supersub == subsc)
  clsDigit {shift.y = -300m};
endif;

endtable;
```

## Exercise 13b

```
#include "stddef.gdh"
table(glyph)
  clsAUpper = U+0041;
  clsVWUpper = (U+0056, U+0057);
endtable;

table(feature)

doKerning {
  id = "k_wv" ;
  name.LG_USENG = string("Kerning");
}

endtable;

table(pos)

if (doKerning)
  clsAUpper { kern.x = -175m } / clsVWUpper _ ;
  clsVWUpper { kern.x = -175m } / clsAUpper _ ;
endif;

endtable;
```

## Exercise 13c

```
#include "stddef.gdh"

table(glyph)
  // same as previous
endtable;

table(feature)

greek_xlit {
  id = "r2gk"
  name.LG_USENG = string("Greek Transliteration");
}

endtable;

table(sub)

if (greek_xlit)
  // rules go here
endif;

endtable;
```

## Exercise 14a

{: .blue-note }
> **Note**
>
> This solution has not been tested. Try at your own risk!

```
#include "stddef.gdh"

table(glyph)
  clsLigComp1 =
    (unicode(0x6f), unicode(0x66), unicode(0x66));  // off
  clsLigComp2 =
    (unicode(0x65), unicode(0x69), unicode(0x6c));  // eil
  // Each component is half the width of the bounding box:
  clsLig = (glyphid(155), // oe
    glyphid(168), // fi
    glyphid(169)) // fl
    {
      comp.c1 = box(bb.left, bb.bottom,    bb.width/2, bb.top);
      comp.c2 = box(bb.width/2, bb.bottom, bb.right, bb.top)
    };

endtable;

table(feature)

lig {
  id = "ligs";
  name.LG_USENG = string("Ligatures");
}

endtable;

table(sub)

if (lig)
  // Note: we use the second character as the selector of
  // the replacement ligature, since it can unambiguously 
  // identify it
  clsLigComp1  clsLigComp2  >
       _  clsLig:(1 2) { comp.c1.ref = @1; comp.c2.ref = @2 };
endif;

endtable;
```

## Exercise 14b

```
#include "stddef.gdh"

table(glyph)
  gNum1 = unicode(0x31);  // 1
  gNum3 = unicode(0x33);  // 3
  clsDenom1 = (unicode(0x34), unicode(0x32));  // 4,2
  clsDenom3 = unicode(0x34); // 4
  gSlash = unicode(0x2f);
  clsLig1 = (unicode(0xbc), unicode(0xbd));  // 1/4, 1/2
  clsLig3 = unicode(0xbe);  // 3/4

  // Each component is half the width of the bounding box:
  clsLig = unicode(0xbc..0xbe)    // 1/4, 1/2, 3/4
    { comp.num = box(bb.left, bb.bottom + 200m, lsb + bb.width*3/8, bb.top);
      comp.slash = box(lsb + bb.width/3, bb.bottom, (lsb + bb.width*2/3, bb.top) ;
      comp.denom = box(lsb + bb.width*5/8,  bb.bottom – 100m, bb.right, bb.bottom + 600m)
    };
endtable;

table(sub)
  // 1/4, 1/2
  gNum1 gSlash clsDenom1  >
      _  _  clsLig1:(1 2 3) {
              comp {num.ref = @1; slash.ref = @2; denom.ref = @3}};
  // 3/4
  gNum3 gSlash clsDenom3  >
      _  _  clsLig3:(1 2 3) {
              comp {num.ref = @1; slash.ref = @2; denom.ref = @3}};
endtable;
```

## Exercise 14c

```
#include "stddef.gdh"

table(glyph)
  one = U+0031;    // 1
  two = U+0032;    // 2
  thr = U+0033;    // 3

  clsNumber = (one, two, thr);

  oneTone = (glyphid(381), glyphid(368), glyphid(354));

  // Two-tone combinations:

  low2 =  (glyphid(381), glyphid(382), glyphid(383));
  mid2 =  (glyphid(367), glyphid(368), glyphid(369));
  high2 = (glyphid(352), glyphid(353), glyphid(354));

  twoTone = (low2, mid2, high2)
    { comp { tone1 = box(0, -descent, aw/2, ascent);
             tone2 = box(aw/2, -descent, aw, ascent) } };
                 
  // Three-tone combinations:
  lowLow3 =   (glyphid(381), glyphid(391), glyphid(392));
  lowMid3 =   (glyphid(446), glyphid(447), glyphid(383));
  lowHigh3 =  (glyphid(450), glyphid(451), glyphid(452));
  midLow3 =   (glyphid(376), glyphid(377), glyphid(378));
  midMid3 =   (glyphid(433), glyphid(368), glyphid(434));
  midHigh3 =  (glyphid(437), glyphid(438), glyphid(439));
  highLow3 =  (glyphid(362), glyphid(363), glyphid(364));
  highMid3 =  (glyphid(352), glyphid(420), glyphid(421));
  highHigh3 = (glyphid(424), glyphid(425), glyphid(354));

  threeTone = (lowLow3,  lowMid3,  lowHigh3,
               midLow3,  midMid3,  midHigh3,
               highLow3, highMid3, highHigh3)
    { comp{ tone1 = box(0, -descent, aw/3, ascent);
            tone2 = box(aw/3, -descent, aw * 2/3, ascent);
            tone3 = box(aw * 2/3, -descent, aw, ascent) } };
endtable;

table(sub)
  // Three-tone sequences:

  one  one  clsNumber  >  _  _  lowLow3:(1 2 3)
    {comp { tone1.ref = @1; tone2.ref = @2; tone3.ref = @3 }};
  one  two  clsNumber  >  _  _  lowMid3:(1 2 3)
    {comp { tone1.ref = @1; tone2.ref = @2; tone3.ref = @3 }};
  one  thr  clsNumber  >  _  _  lowHigh3:(1 2 3)
    {comp { tone1.ref = @1; tone2.ref = @2; tone3.ref = @3 }};

  two  one  clsNumber  >  _  _  midLow3:(1 2 3)
    {comp { tone1.ref = @1; tone2.ref = @2; tone3.ref = @3 }};
  two  two  clsNumber  >  _  _  midMid3:(1 2 3)
    {comp { tone1.ref = @1; tone2.ref = @2; tone3.ref = @3 }};
  two  thr  clsNumber  >  _  _  midHigh3:(1 2 3)
    {comp { tone1.ref = @1; tone2.ref = @2; tone3.ref = @3 }};

  thr  one  clsNumber  >  _  _  highLow3:(1 2 3)
    {comp { tone1.ref = @1; tone2.ref = @2; tone3.ref = @3 }};
  thr  two  clsNumber  >  _  _  highMid3:(1 2 3)
    {comp { tone1.ref = @1; tone2.ref = @2; tone3.ref = @3 }};
  thr  thr  clsNumber  >  _  _  highHigh3:(1 2 3)
    {comp { tone1.ref = @1; tone2.ref = @2; tone3.ref = @3 }};

  // Two-tone sequences:

  one clsNumber  >  _  low2:(1 2)
    {comp { tone1.ref = @1; tone2.ref = @2 }} ;
  two clsNumber  >  _  mid2:(1 2)
    {comp { tone1.ref = @1; tone2.ref = @2 }} ;
  thr  clsNumber  >  _  high2:(1 2)
    {comp { tone1.ref = @1; tone2.ref = @2 }} ;

  // Single tones:

  clsNumber  >  oneTone;

endtable;
```

## Exercise 15

```
#include "stddef.gdh"

Bidi = true;

table(glyph)

  // Not needed; true by default:
  //clsLower = (U+0061..U+007a) { dir = DIR_LEFT };

  clsUpper = (U+0041..U+005a) { dir = DIR_RIGHT };

endtable;

// No rules needed!
```

## Exercise 16

```
#include "stddef.gdh"

table(glyph)
  clsBackRoundedVowel = (unicode(0x6f), unicode(0x75));  // o, u
  clsCons = (unicode(0x62..0x64),  // b..d
    unicode(0x66..0x68),           // f..h
    unicode(0x6A..0x6E),           // j..n
    unicode(0x70..0x74)            // p..t
    unicode(0x76..0x7A));          // v..z
endtable;

table(sub)
  clsCons  clsBackRoundedVowel  >  @2  @1;
endtable;
```

## Exercise 17

```
#include "stddef.gdh"

table(glyph)
  clsBackRoundedVowel = (unicode(0x6f), unicode(0x75));  // o, u
  clsCons = (unicode(0x62..0x64),  // b..d
    unicode(0x66..0x68),           // f..h
    unicode(0x6A..0x6E),           // j..n
    unicode(0x70..0x74)            // p..t
    unicode(0x76..0x7A));          // v..z
endtable;

table(sub)
  _  clsCons  clsBackRoundedVowel  >  @5  @2  _
        / _  _ [clsCons clsCons? ]? _;
endtable;
```

## Exercise 18

```
#include "stddef.gdh"
table(glyph)
  clsBackRndVowelLC = (unicode(0x6f), unicode(0x75)); // o, u
  clsBackRndVowelUC = (unicode(0x4f), unicode(0x55)); // O, U
  clsBackRndVowel = (clsBackRndVowelLC clsBackRndVowelUC);

  clsConsLC = (unicode(0x62..0x64),   // b..d
    unicode(0x66..0x68),              // f..h
    unicode(0x6A..0x6E),              // j..n
    unicode(0x70..0x74),              // p..t
    unicode(0x76..0x7A));             // v..z
  clsConsUC = (unicode(0x42..0x44),   // B..D
    unicode(0x46..0x48),              // F..H
    unicode(0x4A..0x4E),              // J..N
    unicode(0x50..0x54),              // P..T
    unicode(0x56..0x5A));             // V..Z
  clsCons = (clsConsLC clsConsUC);
endtable;

table(sub)
  // Move upper-case from consonant to reordered vowel.
  _  clsConsUC  clsBackRndVowelLC  >  clsBackRndVowelUC$5:5  clsConsLC$2:2  _  // see note below
      /  _  _  [clsCons clsCons? ]?  _;

  // Move upper-case from vowel to consonant (odd, but you never know!).
  _  clsConsLC  clsBackRndVowelUC  >  clsBackRndVowelLC$5:5  clsConsUC$2:2
      / _  _  [clsCons clsCons? ]?  _;

  // Otherwise, case matches on vowel and consonant; just reorder.
  _  clsCons clsBackRndVowel  >  @5  @2  _
      / _ _ [clsCons clsCons? ]? _;
endtable;
```

Note that `clsConsLC$2:2` could be written as `clsConsLC`; the `$2:2` is included only to make the code clearer.

## Exercise 19a

```
#include "stddef.gdh"
table(glyph)
  clsHyphenLike = (unicode(0x2d), unicode(0x003d), unicode(0x5f), unicode(0x7e))
    { break = BREAK_INTRA };
endtable;

// No rules needed!
```

## Exercise 19b

```
#include "stddef.gdh"

table(glyph)
  clsSylBreak = U+002a { break = BREAK_INTRA };
  gHyphen = U+002d;
endtable;

table(sub)
  // Replace an * with a hyphen at a line break, otherwise delete it.
  clsSylBreak  >  gHyphen / _  # {break == BREAK_INTRA};
  clsSylBreak  >  _;
endtable;
```

## Exercise 19c

```
#include "stddef.gdh"

table(glyph)
  clsConsStop = (unicode(0x70), // p
    unicode(0x62),              // b
    unicode(0x74),              // t
    unicode(0x64),              // d
    unicode(0x6b),              // k
    unicode(0x67));             // g
  gHyphen = unicode(0x2d);
endtable;

table(lb)
  clsConsStop {break = BREAK_INTRA}  /  _  clsConsStop;
endtable;

table(sub)
  _  >  gHyphen:1  /  clsConsStop { break == BREAK_INTRA }  _  #;
endtable;
```

## Exercise 19d

Modifying 19a:

```
#include "stddef.gdh"

table(glyph)
  clsHyphenLike = (unicode(0x2d), unicode(0x003d), unicode(0x5f), unicode(0x7e))
    { break = BREAK_INTRA };
endtable;

table(sub)
  // Insert a copy of the "hyphen" at the beginning of the following line.
  _  >  @1  /  clsHyphenLike  #  _;
endtable;
```

Modifying 19b:

```
#include "stddef.gdh"

table(glyph)
  clsSylBreak = U+002a { break = BREAK_INTRA };
  gHyphen = U+002d;
endtable;

table(sub)
  // Replace an * with a hyphen at a line break;
  // also insert a hyphen at the beginning of the following line.
  clsSylBreak  _  >  gHyphen  gHyphen:1
    /  _  # {break == BREAK_INTRA}  _;

  // Otherwise delete it.
  clsSylBreak  >  _;
endtable;
```

Modifying 19c:

```
#include "stddef.gdh"
table(glyph)
  clsConsStop = (unicode(0x70), // p
    unicode(0x62),              // b
    unicode(0x74),              // t
    unicode(0x64),              // d
    unicode(0x6b),              // k
    unicode(0x67));             // g
  gHyphen = unicode(0x2d);
endtable;

table(lb)
  clsConsStop {break = BREAK_INTRA}  /  _  clsConsStop;
endtable;

table(sub)
  // A bug in the Graphite engine forces us to test the
  // breakweight of the line-break itself, not the consonant.
  _  _  >  gHyphen:1  gHyphen:1
    / clsConsStop  _  # { break == BREAK_INTRA }  _;
endtable;
```