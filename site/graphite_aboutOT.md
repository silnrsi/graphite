---
layout: default
title: Graphite and OpenType
nav_order: 20
parent: About Graphite
has_children: true
has_toc: false
---

# Graphite and OpenType

OpenType is the most prevalent smart font technology. Given the extra costs of supporting Graphite in addition to OpenType, what are the gains?

Graphite and OpenType are not, strictly speaking, competing technologies. Applications may support both Graphite and OpenType rendering and fonts may be developed that work with both Graphite and OpenType. The application chooses which rendering approach to use for such a font.

Graphite and OpenType take different approaches to the question of smart font shaping. OpenType endeavors to save description effort within a font by implementing much of the script behavior in code, and the font just describes the font-specific details of which glyph maps to which glyph and what precise positioning to use. Graphite takes the approach of storing all of the description in the font. This allows the Graphite engine to be script-agnostic, and places the responsibility and authority for shaping completely in the hands of the font developer. These two approaches reflect the target uses for the two technologies. Graphite is designed to maximize flexibility, in order to ease the addressing of as yet unknown writing system needs of the many minority languages in the world which do not yet have computer support. OpenType is designed to ease the production of fonts for large linguistic markets and well-established and supported languages for which there is a large user base.

While the needs of minorities are economically small, they are real. There are a number of scripts that are liable never to get established OpenType support. Examples among living languages include Tai Tham, Tai Viet, Chakma. Then there are those which are primarily of academic interest, and these are even less likely to be supported: Kharoshti, Linear B, Music, etc. It is also the case that as more is understood about the needs of particular minority groups, the font developer needs to add extra facilities to their fonts. These needs arise out of discussions with users as they interact with existing implementations. If the time between making a comment and being able to interact with a modified implementation is too long, that discussion breaks down and development does not happen or slows dramatically. Even more established scripts can have problems with the needed rendering for minority languages. This is evidenced with Thai, where there are regular complaints that certain sequences needed for minority languages are identified as illegal for the Thai script, when in fact they are only inappropriate for the Thai language.

## Adding Behavior

Graphite is designed for flexibility of writing system description. Therefore, if an application supports Graphite, it immediately allows for a font developer to create a font to support any script behavior they need without having to change the application or any supporting library.

This is a significant gain. Suppose a new script behavior were discovered as being needed for a script. For example, in the Dravidian languages of South India (for example Betta Kurumba using Malayalam script, Pal Kurumba using Tamil script, Muduga using Kannada script) there is a need to express a vowel as following a consonant rather than over it. This is achieved by storing a halant following it, as per the Malayalam language.

This behavior is not yet supported in many OpenType engines. In fact, there are many different implementations of OpenType: Microsoft, Apple, Adobe, Google and IBM all have their own implementations with their own script behavior programmed into them. To add a new behavior to an OpenType font requires all those implementations to be upgraded and for everyone to agree to implementing the new behavior. In addition, each of these implementations has its own development cycle that can mean a delay of several years before the user is able to render their language appropriately. On the other hand, the Graphite development cycle can result in users being able to work with their language within minutes, or on a font release cycle, which tend to be much shorter, especially where direct support of a language group is a value to the font developer.

Graphite allows a font developer to specify all the behavior of a script. This is useful in two main areas of typesetting. The first, as mentioned, is to get appropriate rendering for a particular minority language or script. The other is for those wishing to do complex typesetting that is not supported by OpenType. This includes things like positioning based on the positions and sizes of other glyphs once positioned, or complex contextualization including line endings (where the application calls for it).

In addition, Graphite rendering is only dependent on what is in the font, and so there is no variation in rendering across different engines or implementations. This is not the case for OpenType. While all the different implementations try to do the best they can, in the "edge cases" of script support, different engines will approach things in different ways. In practice this can mean that a font cannot be made to render correctly in all OpenType engines. It also requires that a font is tested in numerous different engines. This greatly increases the costs of OpenType font development.

## User Defined Features

Just as someone may select some text and make it bold or italicize it, smart fonts also make it possible to change other features of the rendering. Perhaps a different style of the letter 'a' is called for, or diacritics should be flattened to take less space. OpenType has a registered set of these features that an application can set when rendering text. Graphite uses an open-ended approach whereby the names, identifiers and effects of each feature is described in the font. Applications are provided a generic approach to allowing the user to choose which features to set and to specify their values. This allows a font developer to give users all kinds of interesting controls over the rendering of the font without having to go through a registration process first and then lobby for those features to get application support.

## Technical Comparison

The article below is a technical comparison of the performance of Graphite and OpenType, focusing on Nastaliq-style Arabic script.

[Comparison of Graphite and OpenType shaping speeds in a Nastaliq context](graphite_otcompare)