
#include <graphiteng/Font.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    gr_uint16 i;
    gr_uint16 langId = 0x0409;
    gr_uint32 lang = 0;

    GrFace *face = make_file_face(argv[1]);
    if (!face) return 1;
    if (argc > 2) lang = gr_str_to_tag(argv[2]);
    printf("%X\n", lang);
    GrFeatureVal *features = face_featureval_for_lang(face, lang);
    if (!features) return 2;
    int num = face_n_fref(face);
    for (i = 0; i < num; ++i)
    {
        const GrFeatureRef *fref = face_fref(face, i);
        gr_uint32 length = 0;
        char *label = fref_label(fref, &langId, gr_utf8, &length);
        printf("%s\n", label);
        label_destroy(label);
        gr_uint16 val = fref_feature_value(fref, features);
        int numval = fref_n_values(fref);
        int j;

        for (j = 0; j < numval; ++j)
        {
            if (fref_value(fref, j) == val)
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

