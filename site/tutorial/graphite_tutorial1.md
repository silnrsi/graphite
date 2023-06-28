---
layout: default
title: Graphite Tutorial Unit 1
nav_order: 1
parent: Graphite Tutorial Without Graide
nav_exclude: true
---

# Unit 1: Compiling, installing, and testing a Graphite font

[Exercise](graide_tutorial1#exercise-1)

## Discussion

**Compiling.** To compile a GDL program, run grcompiler and include the name of the GDL program as an argument, as well as the name of the font against which to perform the compilation. It is a good idea to include the “-D” switch so that the compiler will output files that can be used for debugging your Graphite font behavior. See “Compiler Debug Files” for detailed information about what is included in these files. It is also possible to specify a new font face name.

The syntax needed to run grcompiler is the following, where the square brackets indicate optional items:

```
grcompiler [-D] gdl-program  original-font-file [new-font-file [new-font-name]]
```

Version 4.2 of the compiler allows you to recompile a font that already contains Graphite tables. When using a previous version of the compiler, the GDL program should always be compiled against _a non-Graphite-enabled font_.

If the name of the new font file is not specified, the name will be the same as the original with “_gr” appended before the “.ttf” extension.

The compiler outputs a file called “gdlerr.txt” containing any error messages or warnings resulting from the compilation. If errors are encountered, the Graphite-enabled version of the font will not be produced.

**Installing.** If you are using Windows XP, Vista or Windows 7, you can simply install the font by dragging it into the Fonts directory, or by right-clicking and choosing Install.

During the compile-test-modify development cycle, Windows XP may require you to explicitly uninstall a font before installing a new version with the same name.

**Testing.** You will be using the WorldPad program to test your Graphite font. To do so, download WorldPad and install it on your system, then run WorldPad.exe. Use the Tools-Writing System Properties dialog to create a writing system that uses your Graphite font for rendering. To do this, add a new writing system and set the font field to the name of your Graphite font. (In earlier versions of WorldPad, you may also need to select Graphite as the renderer.)

In the main WorldPad text-editing window, use the writing system control or the Font-Writing System menu option to select the newly created writing system.

An option is available to have the Graphite engine output log of the transduction process it uses as it performs Graphite rendering. To turn this option on, go to Tools-Options, and check the box labeled “Output Graphite debug log.” For more information about the log, see the “Transduction Log.doc” file.

## Exercise 1

