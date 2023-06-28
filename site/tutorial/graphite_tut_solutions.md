---
layout: default
title: Solutions to Tutorial Exercises
nav_order: 99
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
