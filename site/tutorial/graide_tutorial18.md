---
layout: default
title: Unit 18
nav_order: 180
parent: Graphite Tutorial
grand_parent: Developers
---

{: .tut-nav-bar }
|  [&#x25C0; Unit 17: Optional items](graide_tutorial17) | [&#x25B2; Contents](../graide_tutorial#contents) | [Conclusion &#x25B6;](graide_tutorial_end) |

# Unit 18: Corresponding classes revisited

[Exercise](graide_tutorial18#exercise-18)

## Discussion

In Unit 4 we saw that Graphite uses the mechanism of corresponding classes to select glyphs for output. In other words, the members of the input and output classes are assumed to correspond, and the glyph to be output is the one that is at the same position within the output class as the input glyph is within the input class.

This idea can be extended to glyphs that are in different slots in the rule. The dollar sign ($) followed by a slot number indicates which slot in the input is to be used for the purposes of determining the relevant item position.

For instance, suppose in your font you have different versions of a certain lower diacritic depending on whether it is to be positioned below a base glyph with a descender or without. We can use the corresponding-class approach by creating two classes of glyphs, one containing the list of base glyphs and another containing the corresponding diacritics.

```
table(glyph)
  clsBase = unicode(0x61..0x7a);
  clsDiacPositioned = (gNormal, gNormal, gNormal,  // abc
      gNormal, gNormal, gNormal, gLowered,         // defg
      gNormal, gNormal, gLowered, ...);            // hij…
endtable;
```

The two instances of `gLowered` in the `clsDiacPositioned` class correspond to the characters g and j, which have descenders.

Then the following rule is used to select the proper form of the diacritic, depending on the preceeding base character:

```
clsBase  clsDiac   >   @1  clsDiacPositioned$1;
```

Suppose the base glyph in the input is g. The $1 syntax tells us to determine which item g is within its input class (`clsBase`). The answer is 7, and so the seventh item of class `clsDiacPositioned` - which is `gLowered` - is selected as the glyph to put into the output in the place of the original diacritic. On the other hand, suppose the base glyph in the input is d. Again, the `$1` syntax tells us to determine which item d is within the `clsBase` class. The answer is 4, and so the fourth item of class `clsDiacPositioned` - `gNormal` - placed in the output.

In both cases, `@1` causes the base character itself to be passed through unchanged.

## Exercise 18

Extend your program from Exercise 17 to adjust the capitalization of the reordered items. If the original consonant was uppercase, the reordered vowel should be uppercase in the rendering, and the consonant should become lowercase. In other words, “Cup” is rendered as “Ucp”.

[Solution](graphite_tut_solutions#exercise-18)

{: .tut-nav-bar }
|  [&#x25C0; Unit 17: Optional items](graide_tutorial17) | [&#x25B2; Contents](../graide_tutorial#contents) | [Conclusion &#x25B6;](graide_tutorial_end) |
