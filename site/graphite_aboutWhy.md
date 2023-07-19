---
layout: default
title: Why was Graphite developed?
nav_order: 10
parent: About Graphite
---

# Why was Graphite developed?

 [&#x2197; SIL International](https://sil.org){:target="_blank"} is a non-profit organization that works with ethnolinguistic minority communities as they build their capacity for the development of their own languages. SIL's work encompasses areas such as language development, research, training, materials development, translation, and advocacy. Members of SIL are currently working with approximately one thousand language groups on six continents. Increasingly, many of the areas in which we work use scripts that require complex rendering.

Traditionally, much of our work has been with preliterate groups, or those that are literate only in a national or trade language. In cases where there is no written form for the target language, orthography development is a significant part of our field workers’ literacy promotion efforts. There is a variety of linguistic, sociological, cultural, and political factors that play into the orthography issue, and often it is necessary to adapt the script of the national language for use with these lesser-known languages.

A country's lesser-known languages are usually quite distinct from the national language, much less closely related in a linguistic sense than what one might think of as a “dialect.” Therefore it is common to find linguistic phenomena in the lesser-known language that are not present in the national language and which considerably complicate the issue of orthography development. For example, it may be the case that a lesser-known language is tonal, while the national language is not, and the orthographic solution involves using the standard writing system with some extra diacritics to indicate tone. Or it might have a set of sounds characterized by a certain linguistic feature, such as aspiration or breathiness, that are not present in the national language, and the desire is to add to the standard orthography a set of variant characters to represent these variant sounds.

So computer implementations of writing systems for many lesser-known languages require processing that is not supported in implementations developed for the better-known languages. Some languages use scripts that have not been implemented at all, and must be represented entirely using the Unicode Private Use Area characters. Others require a handful of special characters with complex behaviors, or have special rules about how diacritics are positioned, or how existing characters combine.

To handle the needs of the wide variety of linguistic communities where SIL works, then, we needed an extensible mechanism adequate to handle any orthographic phenomenon we might encounter in any writing system based on any modern script. The only other modern extensible smart font technology adequate to handle all the world’s writing systems is Apple's AAT, and Macs are less accessible for use in many of the communities in which SIL works. So Graphite was originally developed specifically to meet this need on Windows, the platform used by most of our field workers. Since then Graphite has been ported to Linux, where it fills a similar role on that platform. It also became available on Mac beginning with OS 10.6.