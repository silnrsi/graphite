---
layout: default
title: Unit 6
nav_order: 60
parent: Graphite Tutorial
grand_parent: Developers
---

{: .tut-nav-bar }
| [&#x25C0; Unit 5: Deletion and insertion](graide_tutorial5) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 7: Glyph attributes &#x25B6;](graide_tutorial7) |

# Unit 6: Context

[Exercises](graide_tutorial6#exercise-6a)

## Discussion

Often it is desirable to include items in your rule that are not modified by the rule but are simply used for matching. These items are specified in the rule context. The syntax for the rule then becomes:

```
left-hand-side  >  right-hand-side  /  context;
```

Each item from the left- and right-hand sides is represented by an underscore in the context. (Note that this is a somewhat different meaning of an underscore than we saw in the previous unit for insertions and deletions.)

```
gA  gB  gC  >  gX  gY  gZ  /  gP  _  gQ  _  _  gR;
```

The above rule replaces A with X, B with Y, and C with Z. But it only is fired in the situation where gP, gQ, and gR occur as specified in the context, i.e., ‘P A Q B C R’. Notice that there is a one-to-one correspondence between the items in the left-hand side, the items in the right-hand side, and the underscores in the context.

## Exercise 6a

Write a program to replace every instance of the letter c. Replace soft c’s with s and hard c’s with k. (Use the rule that soft c’s preceed e, i, and y, and hard c’s preceed a, o, u, and consonants.) Make sure that the case of the substituted letter matches that of the original “c”.

[Solution](graphite_tut_solutions#exercise-6a)

### Exploring Graide: Creating a test suite

Choose the Tests tab in the left-hand pane. Graide allows you to store suites of test data as part of your project. The top control in the Tests tab shows the name of the file that holds the tests. The project configuration includes the name of your data file, so clicking on the Configure project button will enable you to change it. If you set it as suggested in Exercise 1, the file should be called **tutorial_tests.xml**. If it is currently empty, set it now.

The second control shows the current test group. There is always a default group called "main".

In the Tests tab, click on the green + button - the one below the empty list of tests - to create a new test. Give your test a descriptive name, say hard and soft c. In the Text field enter: Concentric Circles. This is the data that will be run through Graphite. Click OK. The name of your test should show up in the Tests tab. Double-click the name of the test to run it, or use the Run arrow below. The result should look like **Konsentrik Sirkles**.

[Running a test in the test suite](../assets/images/graide6_1_runSavedTest.png)

## Exercise 6b

Extend your program from Exercise 5b to always insert a lowercase u unless both the “Q” and the following letter are uppercase, in which case an uppercase “U” is inserted.

[Solution](graphite_tut_solutions#exercise-6b)

## Exercise 6c

Extend your Greek transliteration program from Exercise 4c or 5a to replace a sigma with the word-final form (&#x03C2;, U+03C2) where appropriate. Hint: use two rules, the first recognizing the situation where the sigma is _not_ word-final.

[Solution](graphite_tut_solutions#exercise-6c)

{: .tut-nav-bar }
| [&#x25C0; Unit 5: Deletion and insertion](graide_tutorial5) | [&#x25B2; Contents](../graide_tutorial#contents) | [Unit 7: Glyph attributes &#x25B6;](graide_tutorial7) |
