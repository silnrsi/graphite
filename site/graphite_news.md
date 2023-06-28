---
layout: default
title: News
nav_order: 300
---

# News

**27 August 2021** - Version 5.2.1 of the [Graphite compiler](https://github.org/silnrsi/grcompiler) has been released. In addition to one or two obscure bug fixes, the build system was changed to use cmake and the Windows release is now 64-bits.

**21 May 2020** - Version 5.2 of the [Graphite compiler](https://github.org/silnrsi/grcompiler) has been released. It uses ICU 66 and includes support for hidden features, substitution rules in the positioning table, and an -e option to specify the GDL error file.

This version fixed a long-standing bug in that file inclusion will now be relative to the including file. Users who previously did inclusion relative to the current working directory will see changes if the current working director differs from the location of the including file.

Graide version 1.0 is now available.

**1 April 2020** - Version 1.3.14 of the Graphite engine has been released. It includes minor bug fixes, some with security implications, and is API-compatible with previous versions. It supports a new feature which is unlisted font features.

**15 August 2018** - Version 1.3.12 of the Graphite engine has been released. There are no functional changes with this new release, although it does include a roundup of various minor issues and the removal of some previously deprecated code. No known CVEs are associated with any changes in this release.

**5 March 2018** - Version 1.3.11 of the Graphite engine has been released. The primary purpose is to release the various bugs that were fixed as a result of a pentest code review. The review only exposed minor issues. The only other changes are minor bug fixes in the collision avoidance and lz4 decompressor.

**5 May 2017** - Version 1.3.10 of the Graphite engine is released, involving fuzz bug fixes.

**30 March 2017** - Version 1.3.8 of the Graphite engine is released, involving a few bug fixes and small changes.

**10 May 2016** - Version 5.0.2 of the Graphite compiler is now available on the compiler download page. This version supports GDL for automatic collision fixing and generating compressed Graphite tables.

**15 March 2016** - Version 1.3.7 of the Graphite engine is released, including quite a lot of bug fixes.

**29 February 2016** - Version 1.3.6 of the Graphite engine is released, involving only security bug fixes.

**15 October 2015** - The Graphite source code has been moved to a GitHub repository. It can be downloaded from:

[https://github.com/silnrsi/graphite](https://github.com/silnrsi/graphite)

Graide can be found here:

[https://github.com/silnrsi/graide](https://github.com/silnrsi/graide)

**31 August 2015** - Version 1.3.1 of the Graphite2 engine released. The main change regards handling of diacritics when reversing text direction to handle mixed left-to-right/right-to-left situations.

Because most applications handle the bidi algorithm at the paragraph level, and this is really necessary to handle edge cases, the engine's internal bidi algorithm will soon be deprecated.

**4 August 2015** - Version 1.3.0 of the Graphite2 engine released. It includes support for automatic collision fixing and compressed Graphite tables.

**30 May 2014** - Version 1.2.4 has been stable for six months, and is now considered the official release for the indefinite future.

Download source code:

[Palaso Graphite2 site](http://projects.palaso.org/attachments/download/407/graphite2-1.2.4.tgz)

[SourceForge Graphite site](http://sourceforge.net/projects/silgraphite/files/graphite2/graphite2-1.2.4.tgz/download)

**29 Nov 2013** - Version 1.2.4 of the Graphite2 engine released. This version is considered complete and, if it proves stable, will serve as the official release for the indefinite future.

**26 June 2012** - Version 4.2 of the Graphite compiler has been released for Windows. Changes include support for a large number of glyph attributes and some improvements for the Graphite2 engine; see the [Readme](https://scripts.sil.org/GraphiteCompilerDownload#compiler_readme) for complete details.

[Download Graphite compiler 4.2](https://scripts.sil.org/GraphiteCompilerDownload)

**13 March 2012** - Firefox 11, which includes Graphite support, has been released.

[Download Firefox 11](http://www.mozilla.org/en-US/firefox/new/)

Graphite must be enabled to work in Firefox. See [Using Graphite in Mozilla Firefox](graphite_firefox).

**6 February 2012** - Version 1.1.0 of the Graphite2 engine source code has been released.

**4 October 2011** - Version 4.1 of the Graphite compiler has been released. It handles mirror attributes for bidirectional text, increases the number of glyph attributes permitted, and creates certain associations automatically.

**July 2011** - LibreOffice 3.4 has been released with Graphite support.

**December 2010** - A new version of the Graphite engine has been released, called Graphite2. It represents a considerable optimization over the earlier engine, and includes a new API that is intended to provide improved compatibility with the text layout approach used by the majority of application software.
