---
layout: default
title: Font Development
nav_order: 30
parent: Developers
---

# Font Development

## Overview

Adding Graphite support to a font involves:

1. writing a program using Graphite Description Language (GDL) describing the behavior of the font
2. compiling the program along with the font to create the Graphite-enabled version of the font
3. testing and debugging your smart font using a Graphite-enabled application

{: .blue-note }
> To add Graphite support to a font, the font's license must grant you permission to modify it.

### 1. The GDL program

GDL is a programming language that describes the "smart" behavior of the font. It is written to correspond to a specific font, and includes definitions of the glyphs in the font and rules describing their behavior.

[Download GDL documentation](GDL.pdf){: .btn .btn-blue }

The following tools can be used to assist in the development of a GDL program:

* make_gdl.pl - a Perl utility that automatically generates some of the necessary GDL code from a font and an XML file of attachment point data. This is included in the SIL [&#x2197; FontUtils](https://scripts.sil.org/FontUtils){:target="_blank"} package.
* FLWriteXml.py - a Python utility that outputs a file of attachment points from FontLab, which can then be used by make_gdl to define attachment points for Graphite. This file can be downloaded from the [&#x2197; SIL Font Utility GitHub repository](https://github.com/silnrsi/pysilfont/tree/master/scripts){:target="_blank"}.

### 2. Compiling the program

The Graphite compiler takes as input the original font and the GDL program, and outputs a new version of the font with the GDL code converted into special-purpose TrueType tables. These tables are used by the Graphite engine to perform smart rendering.

The Windows executable can be downloaded [&#x2197; here](https://scripts.sil.org/GraphiteCompilerDownload){:target="_blank"}.

For Linux systems, the source for the grcompiler program is available from [&#x2197; here](http://sourceforge.net/projects/silgraphite/files/grcompiler){:target="_blank"}. If you are using Ubuntu Linux 12.04, binary packages are available for [64-bit](http://packages.sil.org/ubuntu/pool/main/g/grcompiler/grcompiler_4.2-1+precise1_amd64.deb){:target="_blank"} and [32-bit](http://packages.sil.org/ubuntu/pool/main/g/grcompiler/grcompiler_4.2-1+precise1_i386.deb){:target="_blank"} systems.

### 3. Testing the font

You can test your font using any [Graphite-enabled application](graphite_apps). In order to do so, the most recently compiled version of the font must be properly installed according to the requirements of the operating system. You must also have a way to provide the specific Unicode characters--a keyboard, existing datafile, etc.

If you discover errors in the behavior of the Graphite font, modify the GDL program, recompile, reinstall the output font, and retest.

## Graide

[Graide](graide) is a Graphite font creation tool that is currently under development. Version 1.0 is available.

## Tutorial

The following is a tutorial to help you learn to program using GDL. It includes instructions for using Graide (GRAphite Integrated Development Environment):

[GDL Tutorial](graide_tutorial)

An older version is available that does not use Graide.

[GDL Tutorial Without Graide](graphite_tutorial_wo_graide)

## GDL code snippets

The following small examples can be a useful reference as you develop your Graphite font.

[Code snippets](graphite_codeSnippets)