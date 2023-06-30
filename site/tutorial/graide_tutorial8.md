---
layout: default
title: Unit 8
nav_order: 80
parent: Graphite Tutorial
grand_parent: Developers
---

{: .tut-nav-bar }
|  [&#x25C0; Unit 7: Glyph attributes](graide_tutorial7) | [&#x25B2; Contents](../graide_tutorial#contents) | [Intermission &#x25B6;](graide_tutorial8a) |

# Unit 8: Slot attributes

## Discussion

Unlike glyph attributes, which are constant for every instance of a given glyph, the values of slot attributes may differ for each instance of a glyph. It is the glyph’s role as a slot within the context of the entire stream of text that determines the values of its slot attributes.

Slot attributes are set within rules. Most slot attributes are system-defined (and will be discussed further on), but there is also a set of user-definable (that is, programmer-definable) slot attributes, called user1, user2, etc. More meaningful names can be given to the user-definable attributes using #define.

Slot attributes can be tested in the context of the rule in the same way that glyph attributes are.

{: .tut-nav-bar }
|  [&#x25C0; Unit 7: Glyph attributes](graide_tutorial7) | [&#x25B2; Contents](../graide_tutorial#contents) | [Intermission &#x25B6;](graide_tutorial8a) |
