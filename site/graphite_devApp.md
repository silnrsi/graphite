---
layout: default
title: Adding Graphite to Your Application
nav_order: 70
parent: Developers
---

# Adding Graphite to Your Application
 
## Graphite system overview

The Graphite system consists of:

* Graphite Description Language (GDL) - a rule-based programming language that is used to describe the behavior of a writing system
* the Graphite compiler - used to compile the GDL program along with a font to generate the Graphite-enabled version of the font
* the Graphite engine - uses the Graphite font to assist a text-processing application in performing text layout

[Read more...](graphite_techAbout)

## Graphite2

The Graphite2 engine represents a rework of the original engine developed for the Graphite package. Implementation began in 2010. It is the package used by LibreOffice 3.4 (and later) and Firefox 11 (and later), and can be integrated into applications on the Android versions 2.2 - 2.3.4.

The Graphite2 engine can be downloaded from the [GitHub release page](https://github.com/silnrsi/graphite/releases){:target="_blank"}.

The source code is located on Git Hub: [https://github.com/silnrsi/graphite](https://github.com/silnrsi/graphite){:target="_blank"}

A manual for integrating Graphite2 into an application:

DOWNLOAD GRAPHITE2 MANUAL

## SilGraphite

> **Note**
>
> SilGraphite is deprecated. Further development on the engine is not expected, and it is possible that it may not support fonts developed in the future.

The original Graphite engine, packaged under the name SilGraphite, is used by OpenOffice, Fieldworks, and WorldPad.

The source code can be downloaded from: [SilGraphite SourceForge project](http://sourceforge.net/projects/silgraphite/){:target="_blank"}.

DOWNLOAD SILGRAPHITE DOCUMENTATION

DOWNLOAD GRAPHITE APPLICATION PROGRAMMER'S GUIDE

(The documents above describe version 2 of the Graphite API; the original API is obsolete.)