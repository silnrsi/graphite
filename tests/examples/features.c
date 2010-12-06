
#include <graphiteng/Font.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    gr_uint16 i;
    gr_uint16 langId = 0x0409;
    gr_uint32 lang = 0;
    char idtag[5] = {0, 0, 0, 0, 0};                                /*<1>*/

    GrFace *face = make_file_face(argv[1]);
    if (!face) return 1;
    if (argc > 2) lang = gr_str_to_tag(argv[2]);
    GrFeatureVal *features = face_featureval_for_lang(face, lang);  /*<2>*/
    if (!features) return 2;
    int num = face_n_fref(face);
    for (i = 0; i < num; ++i)
    {
        const GrFeatureRef *fref = face_fref(face, i);              /*<3>*/
        gr_uint32 length = 0;
        char *label = fref_label(fref, &langId, gr_utf8, &length);  /*<4>*/
        printf("%s ", label);
        label_destroy(label);
        gr_uint32 id = fref_id(fref);                               /*<5>*/
        if (id <= 0x00FFFFFF)
            printf("(%d)\n", id);
        else
        {
            gr_tag_to_str(id, idtag);
            printf("(%s)\n", idtag);
        }
        gr_uint16 val = fref_feature_value(fref, features);
        int numval = fref_n_values(fref);
        int j;

        for (j = 0; j < numval; ++j)
        {
            if (fref_value(fref, j) == val)                         /*<6>*/
            {
                label = fref_value_label(fref, j, &langId, gr_utf8, &length);
                printf("\t%s (%d)\n", label, val);
                label_destroy(label);
            }
        }
    }
    face_destroy(face);
    return 0;
}

