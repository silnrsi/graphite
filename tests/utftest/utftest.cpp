#include <graphite2/Segment.h>
#include <stdio.h>

struct test
{
    int len,
    	error;
    unsigned char str[12];
};
struct test tests[] = {
    { 4, -1, {0x7F, 0xDF, 0xBF, 0xEF, 0xBF, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF, 0,    0} },   // U+7F, U+7FF, U+FFFF, U+10FFF
    { 2,  3, {0x7F, 0xDF, 0xBF, 0xF0, 0x8F, 0xBF, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF, 0} },   // U+7F, U+7FF, long(U+FFFF), U+10FFF
    { 1,  1, {0x7F, 0xE0, 0x9F, 0xBF, 0xEF, 0xBF, 0xBF, 0xF4, 0x8F, 0xBF, 0xBF, 0} },   // U+7F, long(U+7FF), U+FFFF, U+10FFF
    { 0,  0, {0xC1, 0xBF, 0xDF, 0xBF, 0xEF, 0xBF, 0xBF, 0xF4, 0xBF, 0xBF, 0xBF, 0} },   // long(U+7F), U+7FF, U+FFFF, U+10FFF
    { 4, -1, {0x01, 0xC2, 0x80, 0xE0, 0xA0, 0x80, 0xF0, 0x90, 0x80, 0x80, 0,    0} },   // U+01, U+80, U+800, U+10000
    { 1,  1, {0x65, 0x9F, 0x65, 0x65, 0,    0,    0,    0,    0,    0,    0,    0} },   // U+65 bad(1) U+65 U+65
    { 2,  2, {0x65, 0x65, 0xC2, 0xC2, 0x65, 0x65, 0,    0,    0,    0,    0,    0} },   // U+65 U+65 bad(1) bad(1) U+65 U+65
    { 2,  2, {0x65, 0x75, 0xE3, 0x84, 0x75, 0x75, 0,    0,    0,    0,    0,    0} },   // U+65 U+75 bad(2) U+75 U+75
    { 2,  2, {0x65, 0x75, 0xF3, 0x84, 0xA5, 0x75, 0x75, 0,    0,    0,    0,    0} },   // U+65 U+75 bad(3) U+75 U+75
    { 2,  2, {0x65, 0x75, 0xF3, 0x84, 0xA5, 0xF5, 0x75, 0,    0,    0,    0,    0} },   // U+65 U+75 bad(3) bad(1) U+75
};

const int numtests = sizeof(tests)/sizeof(test);

int main(int argc, char * argv[]) {
    int i;
    const void * error;

    for (i = 0; i < numtests; ++i)
    {
        int res = gr_count_unicode_characters(gr_utf8, tests[i].str, tests[i].str + sizeof(tests[i].str), &error);
        if (tests[i].error >= 0)
        {
        	if (!error)
        	{
				fprintf(stderr, "%s: test %d failed: expected error condition did not occur\n", argv[0], i + 1);
				return (i+1);
        	}
        	else if (ptrdiff_t(error) - ptrdiff_t(tests[i].str) != tests[i].error)
            {
        		fprintf(stderr, "%s: test %d failed: error at codepoint %d expected at codepoint %d\n", argv[0], i + 1, int(ptrdiff_t(error) - ptrdiff_t(tests[i].str)), tests[i].len);
                return (i+1);
            }
        }
        else if (error)
		{
			fprintf(stderr, "%s: test %d failed: unexpected error occured at codepoint %d\n", argv[0], i + 1, int(ptrdiff_t(error) - ptrdiff_t(tests[i].str)));
			return (i+1);
		}
        if (res != tests[i].len)
        {
            fprintf(stderr, "%s: test %d failed: character count failure %d != %d\n", argv[0], i + 1, res, tests[i].len);
            return (i+1);
        }
    }
    return 0;
}
