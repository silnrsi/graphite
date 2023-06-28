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

