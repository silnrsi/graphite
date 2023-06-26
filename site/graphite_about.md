---
layout: default
title: About Graphite
nav_order: 200
has_children: true
has_toc: false
---

# About Graphite

## What is Graphite?

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

[Examples of complex rendering](https://scriptsource.org/cms/scripts/page.php?item_id=entry_detail&uid=lu6terdg9u){:target="_blank"}

## Graphite system overview

The Graphite system consists of:

* a rule-based programming language Graphite Description Language (GDL) that can be used to describe the behavior of a writing system
* a compiler for that language
* a rendering engine that can serve as the layout component of a text-processing application

Graphite renders TrueType fonts that have been extended by means of compiling a GDL program.

Further technical information is available on the [Graphite technical overview page](graphite_techAbout).

## Why was Graphite developed?

SIL International is a non-profit organization that performs linguistic research, literacy development, and translation work among ethnic minorities around the world. Our members are currently working with approximately a thousand language groups on six continents, and increasingly, many of the areas in which we work use scripts that require complex rendering.

[Read more...](graphite_aboutWhy)

## Graphite and OpenType

OpenType is the most prevalent smart font technology. Given the extra costs of supporting Graphite in addition to OpenType, what are the gains?

Graphite and OpenType are not competing technologies. Applications may support both Graphite and OpenType rendering and fonts may be developed that work with both Graphite and OpenType.

[Read more...](graphite_aboutOT)

## Why should I add Graphite to my font?

The following are some benefits of including Graphite support in a font:

* Graphite provides a way to support a complex script that is not yet handled by standard software (ie, OpenType drivers such as Uniscribe, ICU rendering, etc.).
* Graphite permits customized rules for minority languages, and/or serves as a mechanism to easily add them in the future.
* Graphite supports complex behavior that cannot be handled by OpenType, such as positioning based on the positions and/or sizes of neighboring glyphs, etc.
* Using Graphite, it is possible to quickly implement a complex set of rules that are feasible but more difficult to handle in OpenType.
* Graphite permits user-definable features beyond the defined set of OpenType features.
* Graphite provides consistent smart behavior across all applications.

## What is the current state of the project?

Version 1.2.4 of the Graphite2 engine was released in November 2013. We believe this to be a stable and feature-complete version and do not anticipate much new development in the near future. Future changes are more likely to be made to the compiler rather than the engine, although we certainly plan to fix any bugs that appear.

So please be aware that the lack of a recently released version does not indicate a moribund project, but rather a mature and stable product!