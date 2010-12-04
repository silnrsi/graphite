
#include <graphiteng/Segment.h>
#include <stdio.h>

/* usage: ./simple fontfile.ttf string */
int main(int argc, char **argv)
{
    int rtl = 0;                /* are we rendering right to left? probably not */
    int pointsize = 12;         /* point size in points */
    int dpi = 96;               /* work with this many dots per inch */

    char *pError;               /* location of faulty utf-8 */

    GrFace *face = make_file_face(argv[1]);                                 /*<1>*/
    if (!face) return 1;
    GrFont *font = make_font(pointsize * dpi / 72, face);                   /*<2>*/
    if (!font) return 2;
    size_t numCodePoints = count_unicode_characters(kutf8, argv[2], NULL,
                (const void **)(&pError));                                  /*<3>*/
    if (pError) return 3;
    GrSegment *seg = make_seg(NULL, face, 0, 0, kutf8, argv[2], numCodePoints, rtl); /*<4>*/
    if (!seg) return 3;

    Slot *s;
    for (s = seg_first_slot(seg); s; s = slot_next_in_segment(s))           /*<5>*/
        printf("%d(%f,%f) ", slot_gid(s), slot_origin_X(s), slot_origin_Y(s));
    seg_destroy(seg);
    font_destroy(font);
    face_destroy(face);
    return 0;
}
