---
layout: default
title: Graphite Tutorial
nav_order: 50
parent: Developers
has_children: true
has_toc: false
---

# Graphite tutorial

The following exercises are provided to give you an introduction to programming Graphite fonts. It includes an explanation of the fundamentals of the system and basic experience in using its most important smart rendering capabilities.

The following package includes the fonts to be used for the exercises, solution files, and a PDF of the tutorial.

[Download Tutorial Materials](assets/resources/GraphiteTutorialPkg_v6.zip){: .btn .btn-blue }

This tutorial is not intended to be a comprehensive overview of the Graphite system, nor to provide an exhaustive discussion of all the features and syntax of the GDL programming language. For a complete discussion of GDL, see the “Graphite Description Language” document.

[Download GDL.pdf](assets/resources/GDL.pdf){: .btn .btn-blue }

In order to use these tutorials, you will be using the Graide tool (Graide stands for GRAphite Interactive Development Environment). Graide allows you to run the Graphite compiler to create a Graphite enabled font, and test the results of the font using simple test data. It also includes debugging tools to analyze the behavior of your font.

[&#x2197; Download Graide](https://github/silnrsi/graide){: target="_blank" }

If you choose not to try every exercise, you should put priority on the exercises that include a section called “Exploring Graide…”.

When experimenting with Graide, this tutorial assumes that the GDL program you are working with looks very similar to the provided solution. If your program is quite different, it may be helpful to replace your program with the provided solution for the purposes of exploring the features of Graide.

Each unit of the tutorial consists of a short discussion section followed by one or more exercises for you to try.

[Get started: Running, installing, and debugging with Graide](tutorial/graide_tutorial1)

Return to [Graphite font development](graphite_devFont)

## Contents

* [Unit 1: Running, initializing, and debugging with Graide](tutorial/graide_tutorial1)
* [Unit 2: A very simple GDL program](tutorial/graide_tutorial2)
* [Unit 3: The glyph table](tutorial/graide_tutorial3)
* [Unit 4: Corresponding class items](tutorial/graide_tutorial4)
* [Unit 5: Deletion and insertion](tutorial/graide_tutorial5)
* [Unit 6: Context](tutorial/graide_tutorial6)
* [Unit 7: Glyph attributes](tutorial/graide_tutorial7)
* [Unit 8: Slot attributes](tutorial/graide_tutorial8)
* [Intermission](tutorial/graide_tutorial8a)
* [Unit 9: Multiple passes per table](tutorial/graide_tutorial9)
* [Unit 10: Positioning by shifting](tutorial/graide_tutorial10)
* [Unit 11: Glyph metrics](tutorial/graide_tutorial11)
* [Unit 12: Positioning by attachment](tutorial/graide_tutorial12)
* [Unit 13: Features](tutorial/graide_tutorial13)
* [Unit 14: Ligatures (not available in Graide)](tutorial/graide_tutorial14)
* [Unit 15: Bidirectionality](tutorial/graide_tutorial15)
* [Unit 16: Reordering](tutorial/graide_tutorial16)
* [Unit 17: Optional items](tutorial/graide_tutorial17)
* [Unit 18: Corresponding classes revisited](tutorial/graide_tutorial18)

[Solutions](graphite_tut_solutions)