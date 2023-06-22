---
layout: default
title: About Graphite
nav_order: 2
---

# What is Graphite?

Graphite is a package that can be used to create “smart fonts” capable of displaying writing systems with various complex behaviors. A smart font contains not only letter shapes but also additional instructions indicating how to combine and position the letters in complex ways.

Graphite was developed to provide the flexibility needed for minority languages which often need to be written according to slightly different rules than well-known languages that use the same script.

Examples of complex script behaviors Graphite can handle include:

* contextual shaping
* ligatures
* reordering
* split glyphs
* bidirectionality
* stacking diacritics
* complex positioning

[Examples of complex rendering](https://scriptsource.org/cms/scripts/page.php?item_id=entry_detail&uid=lu6terdg9u)

# Graphite system overview

The Graphite system consists of:

* a rule-based programming language Graphite Description Language (GDL) that can be used to describe the behavior of a writing system
* a compiler for that language
* a rendering engine that can serve as the layout component of a text-processing application

Graphite renders TrueType fonts that have been extended by means of compiling a GDL program.

Further technical information is available on the [Graphite technical overview page](graphite_techAbout).
