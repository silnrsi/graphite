---
layout: default
title: Unit 14
nav_order: 140
parent: Graphite Tutorial
grand_parent: Developers
---

{: .tut-nav-bar }
|  [&#x25C0; Unit 13: Features](graide_tutorial13) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 15: Bidirectionality &#x25B6;](graide_tutorial15) |

# Unit 14: Ligatures

[Exercises](graide_tutorial14#exercise-14a)

## Discussion

A special case of deletion involves the creation of ligatures, where two or more items are replaced by a single glyph representing both or all of them. In the case of a ligature, there are visual components of the final glyph that correspond to each of the underlying characters. There are two parts to this process: first, defining the visual components of the ligature, and second, associating the components with the underlying characters. The first part is done in the glyph table, the second in the substitution table.

{: blue-note }
> Note
> 
> Manipulating the visual components of ligatures is not supported in Graphite2, the engine used by Graide, LibreOffice and Firefox. This behavior can only be tested using WorldPad or FieldWorks.

Let’s suppose we want to replace the sequence ‘ae’ with an æ ligature. The left half of the glyph corresponds to the underlying a and the right half to the underlying e. First we define the visual components, which is done using a specially constructed set of glyph attributes in the glyph table. Each component is defined using a statement with the following syntax:

```
component.<name> = box(<left>, <bottom>, <right>, <top>)
```

Note that each component can consist of one rectangular area of the glyph. A useful way to define components to to use glyph metrics, especially the bounding box of the glyph. For our æ ligature, the complete glyph definition would look like the following:

```
gAElig = U+00E6 {
  component.a = box(bb.left, bb.bottom, bb.width/2, bb.top),
  component.e = box(bb.width/2, bb.bottom, bb.right, bb.top) };
```

These statements specify that the left half of the glyph is the “a” component, and the right half is the “e” component. Note that use of the glyph metrics causes each component to be half as wide as the full glyph. Also note that in the syntax `component.a`, `component` is a system-defined term used specifically for ligatures, and `a` is the component name chosen by the GDL programmer.

The second part of the process is to associate the character in the input with the appropriate ligature component. This is done in a substitution rule, using slot attributes with the following syntax:

```
component.<name>.ref = @<slot-number>
```

So a complete rule to create the æ ligature would be as follows:

```
gA  gE  >  gAElig:(1 2) {component {a.ref = @1; e.ref = @2}}  _;
```

Notice that in addition to mapping the “a” and “e” components to the input slots, the associations must be set as in the case of a normal deletion.

An alternate syntax is:

```
gA  gE  >  gAElig:(1 2) {component.a.ref = @1; component.e.ref = @2}  _;
```

## Exercise 14a

Write a program to create ligatures from the sequences “oe”, “fi”, and “fl”.

Use the **SIDDub3.ttf** font; the glyph IDs for the ligatures are 155, 168, and 169.

Hint: which element of the ligature can be used to uniquely identify the ligature?

Extend your program to create the ligatures only if a feature is turned on.

[Solution](graphite_tut_solutions#exercise-14a)

## Exercise 14b

Write a program to substitute fraction glyphs for the sequences ‘1/2’ (U+00BD), ‘1/4’ (U+00BC), and ‘3/4’ (U+00BE). Keeping in mind that ligature components can overlap visually, how will you define the components so that they can be selected and edited? If you choose to allow your components to overlap, what kind of behavior do you observe with WorldPad's selection mechanism?

Hint: you will probably need to use more than one rule due to the fact that neither the numerator nor the denominator identifies the fraction uniquely. Can you still define the component boxes with a single set of statements?

[Solution](graphite_tut_solutions#exercise-14b)

{: .tut-nav-bar }
|  [&#x25C0; Unit 13: Features](graide_tutorial13) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 15: Bidirectionality &#x25B6;](graide_tutorial15) |
