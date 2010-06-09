bin_PROGRAMS = grngfonttest


AM_CFLAGS = $(GR_CFLAGS) -I$(top_srcdir)/../common/
AM_CXXFLAGS = $(GR_CFLAGS) -I$(top_srcdir)/../common/

grngfonttest_SOURCES = grngFontTest.cpp
grngfonttest_CPPFLAGS = -I ../include -I ../src -I ../common
grngfonttest_LDFLAGS = -L ../src/.libs -lgraphiteng

# EXTRA_DIST = $(top_srcdir)/*.h

TEST_FONT=/usr/share/fonts/truetype/ttf-sil-doulos/DoulosSILR.ttf
TEST_TEXT="Ɛ̈ Ɔ̈ Ŋ Ɛ Ɔ ɛ̈ ɔ̈ ɛ̱ Ɛ̱ ɔ̱ Ɔ̱ ɛ̱̈"
check-local: grngfonttest
	rm -rf grSegmentLog.txt
	$(top_builddir)/grfonttest $(TEST_FONT) \"$(TEST_TEXT)\" && test -s grSegmentLog.txt

clean-local:
	rm -rf grSegmentLog.txt
