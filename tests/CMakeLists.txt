project(testing)
include(Graphite)
option(ENABLE_COMPARE_RENDERER "Build tool to compare output of different renderers" ON) 

add_subdirectory(vm)

if (ENABLE_COMPARE_RENDERER)
    add_subdirectory(comparerenderer)
endif (ENABLE_COMPARE_RENDERER)

include_directories(${graphite2_core_SOURCE_DIR})
set(S ${graphite2_core_SOURCE_DIR})

add_library(graphite2-base
    ${S}/FeatureMap.cpp
    ${S}/NameTable.cpp
    ${S}/Sparse.cpp
    ${S}/TtfUtil.cpp
    ${S}/UtfCodec.cpp)

add_library(graphite2-segcache
    ${S}/SegCache.cpp
    ${S}/SegCacheEntry.cpp
    ${S}/SegCacheStore.cpp
    ${S}/Slot.cpp
    ${S}/GlyphFaceCache.cpp
    ${S}/GlyphFace.cpp)

add_subdirectory(segcache)
add_subdirectory(featuremap)
add_subdirectory(nametabletest)
add_subdirectory(examples)
add_subdirectory(grlist)
add_subdirectory(json)
add_subdirectory(endian)
add_subdirectory(utftest)

enable_testing()

fonttest(padauk1 Padauk.ttf 1015 102F 100F 1039 100F 1031 1038)
fonttest(padauk2 Padauk.ttf 1000 103C 102D 102F)
fonttest(padauk3 Padauk.ttf 101e 1004 103a 1039 1001 103b 102d 102f 1004 103a 1038)
fonttest(padauk4 Padauk.ttf 1005 1000 1039 1000 1030)
fonttest(padauk5 Padauk.ttf 1000 103c 1031 102c 1004 1037 103a)
fonttest(padauk6 Padauk.ttf 1000 102D 1005 1039 1006 102C)
# padauk7 can cause an infinite loop, though the text is miss-spelt
fonttest(padauk7 Padauk.ttf 1017 1014 103c 103d 102f)
fonttest(padauk8 Padauk.ttf 1004 103A 1039 1005)
fonttest(padauk9 Padauk.ttf 1004 103A 1039)
fonttest(padauk10 Padauk.ttf 1004 103D 1000 103A -feat kdot=1,wtri=1)
fonttest(padauk11 Padauk.ttf 100B 1039 100C 1031 102C)
fonttest(scher1 Scheherazadegr.ttf 0628 0628 064E 0644 064E 0654 0627 064E -rtl)
fonttest(scher2 Scheherazadegr.ttf 0627 0644 0625 0639 0644 0627 0646 -rtl)
fonttest(scher3 Scheherazadegr.ttf 0627 0031 0032 002D 0034 0035 0627 -rtl)
fonttest(charis1 charis_r_gr.ttf 0069 02E6 02E8 02E5)
fonttest(charis2 charis_r_gr.ttf 1D510 0041 1D513)
fonttest(charis3 charis_r_gr.ttf 0054 0069 1ec3 0075 -feat 'lang=vie')
fonttest(charis4 charis_r_gr.ttf 006b 0361 070)
fonttest(charis5 charis_r_gr.ttf 0020 006C 0325 0065)
fonttest(magyar1 MagyarLinLibertineG.ttf 0031 0035 -feat 210=36)
fonttest(magyar2 MagyarLinLibertineG.ttf 0031 0030 -feat 210=200)
fonttest(magyar3 MagyarLinLibertineG.ttf 0066 0069 0066 0074 0079 002d 0066 0069 0076 0065 -feat 209=3)
fonttest(grtest1 grtest1gr.ttf 0062 0061 0061 0061 0061 0061 0061 0062 0061)

feattest(padauk_feat Padauk.ttf)
feattest(charis_feat charis_r_gr.ttf)
feattest(scher_feat Scheherazadegr.ttf)
