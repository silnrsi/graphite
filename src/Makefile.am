srcdir = $(top_srcdir)/src

lib_LTLIBRARIES = libgraphiteng.la

libgraphiteng_la_LDFLAGS = -version-info 4:0:0

libgraphiteng_la_SOURCES = create.cpp \
    FontFace.cpp \
    Font.cpp \
    Segment.cpp \
    Slot.cpp \
    TtfUtil.cpp

libgraphiteng_la_SOURCES += ../common/FileFont.cpp

EXTRA_DIST = *.h

EXTRA_DIST += ../common/*.h
