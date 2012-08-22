
#include <graphite2/Segment.h>
#include <stdio.h>
#include <stdlib.h>
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

void *getTable(const void *appHandle, unsigned int name, size_t *len)
{
    void *res;
    FT_Face ftface = (FT_Face)appHandle;
    FT_Load_Sfnt_Table(ftface, name, 0, NULL, len);
    if (!*len) return NULL;
    res = malloc(*len);
    if (!res) return NULL;
    FT_Load_Sfnt_Table(ftface, name, 0, res, len);
    return res;
}

void *releaseTable(const void *appHandle, const void *ptr)
{
    free((void *)ptr);
    return NULL;
}

/* usage: ./simple fontfile.ttf string */
int main(int argc, char **argv)
{
    int rtl = 0;                /* are we rendering right to left? probably not */
    int pointsize = 12;         /* point size in points */
    int dpi = 96;               /* work with this many dots per inch */

    char *pError;               /* location of faulty utf-8 */
    gr_font *font = NULL;
    size_t numCodePoints = 0;
    gr_segment * seg = NULL;
    const gr_slot *s;
    gr_face *face;
    FT_Library ftlib;
    FT_Face ftface;
    gr_face_ops faceops = {(gr_get_table_fn)(&getTable), (gr_release_table_fn)(&releaseTable), NULL};

    if (FT_Init_FreeType(&ftlib)) return -1;
    if (FT_New_Face(ftlib, argv[1], 0, &ftface)) return -2;

    
    face = gr_make_face(ftface, &faceops, gr_face_preloadAll);                              /*<1>*/
    if (!face) return 1;
    font = gr_make_font(pointsize * dpi / 72.0f, face);                         /*<2>*/
    if (!font) return 2;
    numCodePoints = gr_count_unicode_characters(gr_utf8, argv[2], NULL,
                (const void **)(&pError));                                      /*<3>*/
    if (pError) return 3;
    seg = gr_make_seg(font, face, 0, 0, gr_utf8, argv[2], numCodePoints, rtl);  /*<4>*/
    if (!seg) return 3;

    for (s = gr_seg_first_slot(seg); s; s = gr_slot_next_in_segment(s))         /*<5>*/
        printf("%d(%f,%f) ", gr_slot_gid(s), gr_slot_origin_X(s), gr_slot_origin_Y(s));
    gr_seg_destroy(seg);
    gr_font_destroy(font);
    gr_face_destroy(face);
    FT_Done_Face(ftface);
    FT_Done_FreeType(ftlib);
    return 0;
}
