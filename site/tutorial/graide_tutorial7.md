---
layout: default
title: Unit 7
nav_order: 70
parent: Graphite Tutorial
grand_parent: Developers
---

{: .tut-nav-bar }
| [&#x25C0; Unit 6: Context](graide_tutorial6) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 8: Slot attributes &#x25B6;](graide_tutorial8) |

# Unit 7: Glyph attributes

[Exercise](graide_tutorial7#exercise-7)

## Discussion

Besides defining classes of glyphs, a further use of the glyph table is to define _glyph attributes_. The values of glyph attributes are constant for every instance of a given glyph (unlike _slot attributes_ which are discussed later). There are several pre-defined glyph attributes, and GDL programmers can define their own.

The most common way of defining a glyph attribute is to include it in the glyph class definition, as follows:

```
clsExample  =  (item1, item2, …, itemN) {attr1 = value};
```

The statement above defines the glyph attribute `attr1` for the members of the `clsExample` class. The value of a glyph attribute must be a number or a boolean.

For the exercise below, you will need to be able to test the value of attributes. This is done within the context of the rule. For instance, the rule below will only fire if the value of attr1 is true for the matched element of clsExample.

```
clsExample  >  clsModified  /  _ {attr1 == true};
```

Note that as in C++, the assignment operator is “=” and the comparison operator is “==”. Also the ! operator means “not”, and “!=” means “not equal”.

## Exercise 7

Rewrite the program from Exercise 6a to use a glyph attribute. Assign each letter a value for the glyph attribute “followsHardC”. Then test this glyph attribute in the rule to decide whether to change the “c” to a “k” or to an “s”.

Does this approach seem natural and elegant, or contrived? :-)

[Solution](graphite_tut_solutions#exercise-7)

### Exploring Graide: failed rules

When you include tests in your rules, some rules may match but not be fired due to the test returning false. These “failed” rules will be displayed in the Rules tab but will be marked “(failed)”.

Try running the data: `city council`. The output should look like: **sity kounsil**. Double-click on Pass 1 to bring up the Rules tab. You will see several rules that are marked “failed”. Notice that there are no green glyphs in these rules’ output, since no changes were made. The previous row will show gray glyphs, indicating glyphs that were matched but were left unchanged.

### Exploring Graide: glyph attributes

The Glyph tab will show any glyph attributes that are set in your GDL program. Again, run the string `city council` and bring up the Rules tab. Select the letter ‘i’ in the first row (either by clicking the black pixels in the glyph or by clicking ‘g_i’ in the third column) and choose the Glyph tab. Notice that there is a glyph attribute listed called ‘followsHardC’ with the value 0 (false). Now select the ‘t’. The same glyph attribute is listed, but the value is 1 (true).

[The Glyph pane showing glyph attributes](graide7_1_glyphAttribute.png)

(If your glyph attributes don’t seem to look the same, it could be because your program is somewhat different than the provided solution.)

{: .tut-nav-bar }
| [&#x25C0; Unit 6: Context](graide_tutorial6) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 8: Slot attributes &#x25B6;](graide_tutorial8) |
