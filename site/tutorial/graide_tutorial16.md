---
layout: default
title: Unit 16
nav_order: 160
parent: Graphite Tutorial
grand_parent: Developers
---

{: .tut-nav-bar }
|  [&#x25C0; Unit 15: Bidirectionality](graide_tutorial15) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 17: Optional items &#x25B6;](graide_tutorial17) |

# Unit 16: Reordering

[Exercise](graide_tutorial16#exercise-16)

## Discussion

Now we return to the substitution table, to discuss yet another behavior it can be used to implement. This is reordering, specifically of the sort that we find in many scripts of South and Southeast Asia.

Here we extend the use of the @ symbol, which we saw briefly in our discussion of ligatures. The @ symbol is used to access an item in a rule. The number following the @ symbol indicates the position of the referenced slot with respect to its position in the context (the @ symbol alone indicates the slot itself).

```
gA  gB  >  @3  @1  /  _  gX  _;
```

The rule above replaces the A with the item at position 3 (the B) and replaces the B with the item at position 1 (the A) when an X occurs between the two characters. In other words, the A and the B switch places. Notice that the slot number for `gB` is not 2, but 3, its position within the entire context.

The `@` symbol can also be used to copy a glyph with no changes, or with changes only to the slot attributes.

## Exercise 16

Write a program to reorder back rounded vowels (o and u) such that they occur before a preceeding consonant. (Remember that reordering is handled in the substitution table.) You may ignore the problem of vowel sequences (unless you feel particularly ambitious).

[Solution](graphite_tut_solutions#exercise-16)

{: .tut-nav-bar }
|  [&#x25C0; Unit 15: Bidirectionality](graide_tutorial15) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 17: Optional items &#x25B6;](graide_tutorial17) |
