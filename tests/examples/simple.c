
#include <graphiteng/Segment.h>
#include <stdio.h>

/* usage: ./simple fontfile.ttf string */
int main(int argc, char **argv)
{
    int rtl = 0;                /* are we rendering right to left? probably not */
    int pointsize = 12;         /* point size in points */
    int dpi = 96;               /* work with this many dots per inch */

    char *pError;               /* location of faulty utf-8 */

    GrFace *face = gr_make_file_face(argv[1], 0);                             /*<1>*/
    if (!face) return 1;
    GrFont *font = gr_make_font(pointsize * dpi / 72, face);                   /*<2>*/
    if (!font) return 2;
    size_t numCodePoints = gr_count_unicode_characters(gr_utf8, argv[2], NULL,
                (const void **)(&pError));                                  /*<3>*/
    if (pError) return 3;
    GrSegment *seg = gr_make_seg(font, face, 0, 0, gr_utf8, argv[2], numCodePoints, rtl); /*<4>*/
    if (!seg) return 3;

    const GrSlot *s;
    for (s = gr_seg_first_slot(seg); s; s = gr_slot_next_in_segment(s))           /*<5>*/
        printf("%d(%f,%f) ", gr_slot_gid(s), gr_slot_origin_X(s), gr_slot_origin_Y(s));
    gr_seg_destroy(seg);
    gr_font_destroy(font);
    gr_face_destroy(face);
    return 0;
}
