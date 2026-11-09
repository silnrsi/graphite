#include <graphite2/Font.h>
#include <string.h>

int main(void)
{
    struct {
        char tag[4];
        char sentinel;
    } out = {{0, 0, 0, 0}, 0x7f};

    gr_tag_to_str(0x46656174, out.tag);

    return memcmp(out.tag, "Feat", 4) != 0 || out.sentinel != 0x7f;
}
