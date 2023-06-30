---
layout: default
title: Unit 15
nav_order: 150
parent: Graphite Tutorial
grand_parent: Developers
---

{: .tut-nav-bar }
|  [&#x25C0; Unit 14: Ligatures](graide_tutorial14) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 16: Reordering &#x25B6;](graide_tutorial16) |

# Unit 15: Bidirectionality

[Exercise](graide_tutorial15#exercise-15)

## Discussion

Some writing systems are bidirectional, meaning that they may consist of ranges of text that are written from right-to-left and other ranges that are written from left-to-right. The Unicode Standard contains an algorithm for rendering bidirectional scripts, as well as mixing scripts of different directions.

This algorithm is built into Graphite. Between the substitution and positioning tables there is a special pass called the “bidi” pass. It takes the output of the final substitution pass and reorders the characters according the bidi algorithm and the characters’ directionality attributes. This stream of reordered characters serves as input to the first positioning pass.

Every Unicode character has a “directionality” property that is used to determine its behavior with respect to the bidirectional algorithm. Within Graphite, the value of this property is accessible as a system-defined glyph attribute, `directionality` (abbreviated `dir`). If you wish to override the directionality of a character, or set it for a PUA character, you can do so by setting the `dir` glyph attribute in the glyph table. There is also a `dir` slot attribute (initialized from the glyph attribute) that can be set by means of rules.

The following are the most common directionality attribute values:

* DIR_LEFT: left-to-right (L)
* DIR_RIGHT: right-to-left (R)
* DIR_ARABIC: Arabic (AR; there are some subtle differences between DIR_ARABIC and DIR_RIGHT)
* DIR_ARABNUMBER: Arabic number (AN - left-to-right embedded in right-to-left)
* DIR_WHITESPACE: whitespace
* DIR_OTHERNEUTRAL: neutral

In order to include a bidi pass in your Graphite font, you must turn it on using the following statement:

```
Bidi = true;
```

## Exercise 15

Write a program to treat lowercase letters as left-to-right and uppercase as right-to-left (DIR_RIGHT). Do this by setting the directionality glyph attribute.

Hint: what is the minimum number of rules that you need?

[Solution](graphite_tut_solutions#exercise-15)

### Exploring user interface issues with bidirectional text

{: .blue-note }
> **NOTE**
>
> You will need to use Libre Office for the following tests.

Observe how insertion-point and range selections behave with mixed-direction text in LibreOffice. What happens to white space at the line boundaries?

Change the paragraph direction to right-to-left; what differences do you observe?

{: .tut-nav-bar }
|  [&#x25C0; Unit 14: Ligatures](graide_tutorial14) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 16: Reordering &#x25B6;](graide_tutorial16) |
