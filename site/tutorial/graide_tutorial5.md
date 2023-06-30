---
layout: default
title: Unit 5
nav_order: 50
parent: Graphite Tutorial
grand_parent: Developers
---

{: .tut-nav-bar }
| [&#x25C0; Unit 4: Corresponding glyph classes](graide_tutorial4) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 6: Context &#x25B6;](graide_tutorial6) |

# Unit 5: Deletion and insertion

[Exercises](graide_tutorial5#exercise-5a)

## Discussion

**Deletion.** The most common use for deletion is to replace two glyphs with one. Strictly speaking, this involves one replacement and one deletion. The deleted slot is indicated by an underscore in the right-hand side (output). For instance, the rule below replaces a base character and a diacritic with a single character containing both the base and the diacritic.

```
clsBase  clsDiac  >  clsBasePlusDiac  _;
```

Notice that there must always be the same number of items (class names or underscores) on the left- and right-hand sides.

Usually when doing a deletion, you are not, strictly speaking, totally deleting a glyph but rather merging it with another. When this is true you need to specify the slot the deleted glyph is being merged with. This is done by explicitly associating the merged glyph in the output with both of the input slots. This makes it possible for the user to select the single visible glyph and by modifying it, modify both of the underlying characters. For instance, to create this association in the above rule, use the following syntax:

```
clsBase  clsDiac  >  clsBasePlusDiac:(1 2)  _;
```

This states that the base-plus-diacritic glyph is associated with slots 1 and 2 in the input, i.e., the base character and the diacritic.

**Insertion.** Insertion is the opposite of deletion. Generally, what occurs is that a single item in the input is replaced by two or more glyphs in the output. In the case of insertion, the underscore occurs in the left-hand side to show the location of the inserted slot. For instance, the rule below is the opposite of the one above:

```
clsBasePlusDiac  _  >  clsBase:1  clsDiac:1;
```

As with deletion, in the case of insertion you are rarely inserting a glyph from thin air, but rather splitting the information from one glyph into two (or more). So you can see above that association is made explicit in this situation as well. We indicate that the two resulting slots are both associated with the single input slot. This has the effect that selecting the single underlying character will cause both surface glyphs to be highlighted.

## Exercise 5a

Revise your Greek program from Exercise 4c to replace ‘th’ with theta (U+03B8), ‘ph’ with phi (U+03C6), and ‘ps’ with psi (U+03C8). Ensure that selecting the single surface glyph will cause both underlying characters to be selected.

Delete any instances of the letters “j” and “v”. (How will you handle the issue of association?)

[Solution](graphite_tut_solutions#exercise-5a)

### Exploring Graide: glyph deletion

Run the following test data: jack in the box. The output should look like: **αχκ ιν θε βοξ**. Bring up the Rules tab for pass 1. Notice that in the first row, the ‘j’ is highlighted in pink, but there is no corresponding green glyph in the second row. This is because the ‘j’ has been deleted. Also in the seventh row, both the ‘t’ and ‘h’ glyphs are highlighted in pink, but in the following row, only the theta (θ) is shown green.

## Exericse 5b

Write a program to automatically insert a “u” following every “q”. Use associations to ensure that selecting the “q” will automatically select the “u” as well.

Compile this program into the DoulosGrTut.ttf font.

[Solution](graphite_tut_solutions#exercise-5b)

{: .tut-nav-bar }
| [&#x25C0; Unit 4: Corresponding glyph classes](graide_tutorial4) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 6: Context &#x25B6;](graide_tutorial6) |
