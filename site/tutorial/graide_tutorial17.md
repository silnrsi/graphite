---
layout: default
title: Unit 17
nav_order: 170
parent: Graphite Tutorial
grand_parent: Developers
---

{: .tut-nav-bar2 }
|  [&#x25C0; Unit 16: Ligatures](graide_tutorial16) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 18: Corresponding classes revisited &#x25B6;](graide_tutorial18) |

# Unit 17: Optional items

[Exercise](graide_tutorial17#exercise-17)

## Discussion

It is possible to specify optional items in a rule. This is done by following the optional item with a question mark. For instance, if we say that the intervening X in the rule from Unit 16 is optional, our rule looks like this:

```
gA  gB  >  @3  @1  /  _  gX?  _;
```

Notice that the indexing of elements must take into account all optional items; hence gB is considered item #3 in the above rule.

Optional markers can be placed either in the left-hand side or the context, but not in the right-hand side.

Ranges of items can be marked optional by using square brackets:

```
gA  gB  >  @3  @1  /  _  [gX gY gZ]?  _;
```

Note that in the rule above, the entire sequence ‘X Y Z’ must occur or none of it is allowed. You can also embed sequences of optional items. In the following rule, X may occur zero to three times:

```
gA  gB  >  @3  @1  /  _  [gX [gX gX? ]? ]?  _;
```

## Exercise 17

Extend your program from Exercise 16 to handle the presence of a consonant cluster before the vowel to be moved; i.e., allow one or two optional consonants between the initial consonant and the vowel.

Hint: instead of switching items, you may need to delete the vowel and reinsert it.

[Solution](graphite_tut_solutions#exercise-17)

### Exploring Graide: the numbering of rules with optional items

Test your program in Graide with data the includes the optional consonants and data that does not, for instance: `so sto stro to tro`. Look at the list of rules in the Rules tab. What do you notice about the numbering of the rules?

Even though there is a single GDL rule that handle the various optional items, the Graphite compiler actually creates multiple rules corresponding to the presence or absence of the optional items. This is reflected in the numbering of the rules in the Rules tab.

{: .tut-nav-bar2 }
|  [&#x25C0; Unit 16: Ligatures](graide_tutorial16) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 18: Corresponding classes revisited &#x25B6;](graide_tutorial18) |
