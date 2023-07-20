---
layout: default
title: Graphite-enabled Fonts
nav_order: 70
parent: Using Graphite
---

# Graphite-enabled fonts

* [Production-quality fonts](graphite_fonts#production-quality-fonts)
    * [Arabic](graphite_fonts#arabic)
    * [Myanmar (Burmese)](graphite_fonts#myanmar-burmese)
    * [Devanagari](graphite_fonts#devanagari)
    * [Tamil](graphite_fonts#tamil)
    * [Tamil Brahmi](graphite_fonts#tamil-brahmi)
    * [Tai Viet](graphite_fonts#tai-viet)
    * [Greek](graphite_fonts#greek)
    * [Lanna](graphite_fonts#lanna)
    * [Coptic](graphite_fonts#coptic)
    * [Ethiopic](graphite_fonts#ethiopic)
    * [Dai Banna](graphite_fonts#dai-banna)
    * [Lepcha](graphite_fonts#lepcha)
    * [Roman script variations](graphite_fonts#roman-script-variations)
    * [Blackletter style for Roman text](graphite_fonts#blackletter-style-for-roman-text)
    * [Cipher Music](graphite_fonts#cipher-music)
    * [Tengwar](graphite_fonts#tengwar)
* [Experimental fonts](graphite_fonts#experimental-fonts)
    * [N'Ko](graphite_fonts#nko)
    * [Sumero-Akkadian Cuneiform](graphite_fonts#sumero-akkadian-cuneiform)
* [Toy fonts](graphite_fonts#toy-fonts)
    * [Simple Graphite Font](graphite_fonts#simple-graphite-font)
    * [Pig Latin](graphite_fonts#pig-latin)
* [Create your own Graphite font](graphite_fonts#create-your-own-graphite-font)

## Production-quality fonts[*](#non-sil)

### Arabic

[&#x2197; Awami Nastaliq](https://software.sil.org/awami/){:target="_blank"} supports a wide variety of languages of Pakistan that are written with the sloping Nastaliq style.

### Myanmar (Burmese)

[&#x2197; Padauk](https://software.sil.org/padauk/){:target="_blank"} was developed by SIL to handle the Burmese script. Its Graphite support is particularly useful for lesser-known languages.

### Devanagari

[&#x2197; Annapurna SIL](https://software.sil.org/annapurna){:target="_blank"} renders Devanagari for languages of north India and Nepal, including support for lesser-known variations.

### Tamil[*](#non-sil)

The [&#x2197; Krishna Tamil](https://sourceforge.net/p/silgraphite/mailman/message/29478357/){:target="_blank"} font is a Graphite-enabled font derived from Lohit Tamil. It handles classical Tamil orthogrphy and old Tamil grammatical sequences.

The [&#x2197; ThiruValluvar](https://github.com/nlci/taml-font-thiruvalluvar/releases){:target="_blank"} font also supports Tamil.

### Tamil Brahmi[*](#non-sil)

The [&#x2193; Adinatha Tamil Brahmi](http://www.virtualvinodh.com/download/Adinatha-Tamil-Brahmi.zip) font provides support for the subset of the historical Brahmi script used in Tamil Nadu.

### Tai Viet

[&#x2197; Tai Heritage Pro](https://software.sil.org/taiheritage){:target="_blank"} supports the Tai Viet script as used for Tai Dam and related languages of Southeast Asia.

### Greek[*](#non-sil)

These fonts also include Latin and Cyrillic.

[&#x2197; Theano Classical Fonts](https://www.fontspace.com/theano-font-f13396){:target="_blank"}

[&#x2197; Old Standard Font Family](https://www.fontsquirrel.com/fonts/old-standard-tt){:target="_blank"}

### Lanna

[&#x2197; Payap Lanna](https://software.sil.org/payaplanna/){:target="_blank"} supports the Tai Tham script of Northern Thailand.

### Coptic

[&#x2197; Sophia Nubian](https://software.sil.org/sophianubian){:target="_blank"} is a font developed for Nubian languages which use the Coptic Unicode character set.

### Ethiopic

The Graphite smarts in [&#x2197; Abyssinica SIL](https://software.sil.org/abyssinica){:target="_blank"} support alternate feature selection of glyph variants as well as gemination combining marks.

### Dai Banna

A set of fonts for the Dai Banna or New Tai Lue script can be found [&#x2197; here](https://software.sil.org/daibanna/){:target="_blank"}.

### Lepcha

Mingzat, a font for the Lepcha script can be found [&#x2197; here](https://software.sil.org/mingzat){:target="_blank"}.

### Roman script variations[*](#non-sil)

[&#x2197; Magyar Linux Libertine](http://numbertext.org/linux){:target="_blank"} is a Graphite-enabled font with ligatures, small caps, old style numbers, proportional or monospaced numbers, automatic thousand separators, minus sign, real superscript and subscript, German umlaut variants and capital eszett, fractions, number to number name conversion (in 23 languages).

### Blackletter style for Roman text[*](#non-sil)

[&#x2197; Unifraktur](https://unifraktur.sourceforge.net/){:target="_blank"}

### Cipher Music

[&#x2197; Doulos SIL Cipher](https://software.sil.org/doulos-sil-cipher){:target="_blank"} supports the cipher music notation system that is used in China and Indonesia. The font is still under development.

### Tengwar[*](#non-sil)

[&#x2197; Tengwar](http://freetengwar.sourceforge.net/){:target="_blank"} is an "Elvish" script developed by J. R. R. Tolkien.

<a name="non-sil"></a>

{: .blue-note }
> **\*Note**
>
> Fonts marked with an asterisk (*) were not developed by SIL and we cannot guarantee their quality or correctness.


## Experimental fonts

### N'Ko

A beta of [&#x2193; Conakry](https://www.evertype.com/fonts/nko/ConakryFont.zip), a font to support the N'Ko script, is available from Evertype.

### Sumero-Akkadian Cuneiform

[Download](assets/resources/CuneiformGraphiteFont.zip){: .btn .btn-blue }



## Toy fonts

Just for fun!

### Simple Graphite Font

Makes all consonants uppercase and all vowels lowercase. Useful for testing Graphite installation.

[Download](assets/resources/SimpleGraphiteFont.zip){: .btn .btn-blue }

### Pig Latin

Generates Pig Latin on the fly! The behavior is slightly incomplete and buggy--feel free to improve!

[Download](assets/resources/PigLatinGraphiteFont.zip){: .btn .btn-blue }

## Create your own Graphite font

> See [resources to help you add Graphite support to your font](graphite_devFont).