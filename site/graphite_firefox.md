---
layout: default
title: Graphite and Firefox
nav_order: 50
parent: Using Graphite
---

# Using Graphite in Mozilla Firefox

## Graphite in Firefox

Versions 11 and later of Mozilla Firefox 11 directly support Graphite on Mac OS X, Linux, Windows and Android platforms. A web page can be rendered using both local and server-based fonts, and specific Graphite font features can be controlled via CSS.

Graphite is automatically activated in Firefox. In early versions this support was not switched on by default and needed to be enabled by the user. If you have a version of Firefox prior to 28, see below for instructions on how to activate it.

To see Graphite in operation, you need a web page uses some CSS referring to a Graphite-enabled font. The font can either be installed on your own system or remotely used as a web font.

Graphite is also supported in Pale Moon, a browser based on a fork of Firefox.

### A menu toggle for Android

An extension is available that provides [&#x2197; a menu toggle for Graphite support in Firefox on Android](https://addons.mozilla.org/en-US/android/addon/toggle-graphite-support){:target="_blank"}.

## Testing Graphite behaviors in Firefox

Now you can test content relying on smart behaviors in a Graphite-enabled font.

Go to [Graphite Font Demo](graphite_fontdemo) to see some sample fonts rendering in your Firefox browser.

## Using Graphite font features in Firefox

It is possible to make use of Graphite font features in an HTML page by specifying them with CSS. To do this you must know the ID of the feature of interest in the font and the value you wish to use. This information should be included in the font documentation.

{: .blue-note }
> **NOTE**
>
> Some earlier Graphite-enabled fonts used numerical feature IDs which will not work in Firefox. The following maintenance releases of SIL's Roman and Cyrillic fonts are available that use 4-character IDs: [&#x2197; Doulos SIL 4.112](https://scripts.sil.org/cms/scripts/page.php?item_id=DoulosSIL_download#4112){:target="_blank"}, [&#x2197; Charis SIL 4.112](https://scripts.sil.org/cms/scripts/page.php?item_id=CharisSIL_download#4112){:target="_blank"}, [&#x2197; Andika 1.004](https://scripts.sil.org/cms/scripts/page.php?item_id=Andika_download#1004){:target="_blank"}, and [&#x2197; Gentium Plus 1.510](https://scripts.sil.org/cms/scripts/page.php?item_id=Gentium_download#1510){:target="_blank"}; later versions of these fonts will also work.

The CSS syntax is:

```
<css-style-name> {
  -moz-font-feature-settings: "<feature-id>" <feature-value>;
}
```

The `<feature-value>` is optional and if omitted will be assumed to be 1.

Note that for Firefox version 14 and earlier, the syntax is:

```
<css-style-name> {
  -moz-font-feature-settings: "<feature-id>=<feature-value>";
}
```

To be on the safe side, you can use these two syntaxes simultaneously.

For instance, if you have a font called "My Graphite Font" and with a feature ID of 'alts', the following would define a CSS style using this font with the feature set to the value 2:

```
.alternate_font_style {
  font-family: "My Graphite Font", <other fallback fonts>;
  /* new syntax: */
  -moz-font-feature-settings: "alts" 2;
  /* old syntax: */
  -moz-font-feature-settings: "alts=2";
}
```

To indicate more than one feature, use a comma as a delimiter:

```
.alternate_font_style {
  font-family: "My Graphite Font", <other fallback fonts>;
  /* new syntax: */
  -moz-font-feature-settings: "fea1" 2, "fea2" 6, "fea3"; /* fea3 is 1 or 'on' */
  /* old syntax: */
  -moz-font-feature-settings: "fea1=2,fea2=6,fea3";
}
```

More complete documentation of feature settings in CSS can be found at [&#x2197; http://dev.w3.org/csswg/css3-fonts/#propdef-font-feature-settings](http://dev.w3.org/csswg/css3-fonts/#propdef-font-feature-settings){:target="_blank"}.

For more detail on how to specify fonts on web pages see [&#x2197; Using SIL Fonts on Web Pages](https://scripts.sil.org/using_web_fonts){: target="_blank"}.

## Supported platforms
This Graphite support has been tested and is known to work on the following platforms:

* Windows 7
* Windows Vista
* Ubuntu 11.10
* Android 3.2
* Mac OS X 10.7.3

## Switching on Graphite in Firefox 11 and 45.0.1

{: .blue-note }
> **Note**
>
> These instructions to not apply to the FirefoxOS in which Graphite is enabled by default and cannot be disabled.

If you have a version of Firefox earlier than 28, or 45.0.1 or later, the Graphite enabling might not be switched on by default, and you need to go into the advanced configuration settings manager to switch it on before you can use it.

Make sure you have a version of  Mozilla Firefox 11 or later installed, then go through the following steps:

* Type `about:config` in your address bar.
* Press _Enter_.
* Click on the _"I'll be careful, I promise"_ button. This big warning is to make sure you really know what you are doing when changing any advanced parameters, as you could easily introduce problems with core configuration settings. We will only make a small change to one Graphite-related parameter. (You should be able to go back and toggle parameters off if needed.)
* Type _graphite_ in your search bar to look for the right parameter. Firefox will show you the following configuration preference name: _gfx.font_rendering.graphite.enabled_. Alternatively, you can just scroll down the page to find an option called _gfx.font_rendering.graphite.enabled_.
* Double-click on the preference name line to set the value from false to true. You will see the whole line becoming bold.
