#pragma once

class Option
{
public:
    typedef enum {
        OPTION_BOOL,
        OPTION_STRING,
        OPTION_INT,
        OPTION_FLOAT
    } OptionType;
    Option(const char * shortOpt, const char * longOpt, const char * desc, OptionType type)
        : m_shortOption(shortOpt), m_longOption(longOpt), m_description(desc),
        m_type(type), m_position(-1)
    {

    }
    ~Option()
    {

    }
    void setPosition(int i) { m_position = i; }
    const char * shortOption() const { return m_shortOption; }
    const char * longOption() const { return m_longOption; }
    const char * description() const { return m_description; }
    bool exists() { return m_position > -1; }
    const char * get(char ** argv) const { return (m_position > -1)? argv[m_position] : NULL; }
    int getInt(char ** argv) const { return (m_position > -1)? atoi(argv[m_position]) : 0; }
    double getFloat(char ** argv) const { return (m_position > -1)? atof(argv[m_position]) : 0.0; }
    OptionType type() const { return m_type; }
private:
    const char * m_shortOption;
    const char * m_longOption;
    const char * m_description;
    OptionType m_type;
    int m_position;
};

// enum must be in same order as rendererOptions
typedef enum {
    OptTextFile,
    OptFontFile,
    OptSize,
    OptGraphite,
    OptGraphiteNg,
    OptHarfbuzzNg,
    OptRtl,
    OptRepeat,
    OptTolerance,
    OptCompare,
    OptLogFile,
    OptVerbose,
    OptAlternativeFont,
    OptIgnoreGlyphIdDifferences,
    OptSegCache,
    OptFeatures
} OptionId;

static Option rendererOptions[] = {
    Option("-t", "--text", "Text file", Option::OPTION_STRING),
    Option("-f", "--font", "Font file", Option::OPTION_STRING),
    Option("-s", "--size", "Font size", Option::OPTION_INT),
    Option("-g", "--graphite", "Use Graphite renderer", Option::OPTION_BOOL),
    Option("-n", "--graphiteng", "Use Graphite NG renderer", Option::OPTION_BOOL),
    Option("-h", "--harfbuzzng", "Use Harfbuzz NG renderer", Option::OPTION_BOOL),
    Option("-r", "--rtl", "Right to left", Option::OPTION_BOOL),
    Option("", "--repeat", "Number of times to rerun rendering", Option::OPTION_INT),
    Option("", "--tolerance", "Ignore differences in position smaller than this", Option::OPTION_FLOAT),
    Option("-c", "--compare", "Compare glyph output", Option::OPTION_BOOL),
    Option("-l", "--log", "Log file for results instead of stdout", Option::OPTION_STRING),
    Option("-v", "--verbose", "Output lots of info", Option::OPTION_BOOL),
    Option("-a", "--alt-font", "Alternative font file", Option::OPTION_STRING),
    Option("", "--ignore-gid", "Ignore Glyph IDs in comparison (use with -c -a alt.ttf)", Option::OPTION_BOOL),
    Option("", "--seg-cache", "Enable Segment Cache", Option::OPTION_INT)
    //Option("", "--features", "Feature list", Option::OPTION_STRING),
    
};

const int NUM_RENDERER_OPTIONS = sizeof(rendererOptions) / sizeof(Option);

void showOptions()
{
    const char * optionTypeDesc[] = {
        "", "arg", "int", "float"
    };
    for (size_t i = 0; i < NUM_RENDERER_OPTIONS; i++)
    {
        fprintf(stderr, "%s %s\t%s\t%s\n", rendererOptions[i].shortOption(),
                rendererOptions[i].longOption(),
                optionTypeDesc[rendererOptions[i].type()],
                rendererOptions[i].description());
    }
}


bool parseOptions(int argc, char ** argv)
{
    for (int i = 1; i < argc; i++)
    {
        bool known = false;
        for (int j = 0; j < NUM_RENDERER_OPTIONS && (!known); j++)
        {
            if ((strcmp(argv[i], rendererOptions[j].longOption()) == 0) ||
                ((strlen(rendererOptions[j].shortOption()) > 0) &&
                 (strcmp(argv[i], rendererOptions[j].shortOption()) == 0)))
            {
                known = true;
                if (rendererOptions[j].type() > Option::OPTION_BOOL)
                {
                    if (argc > i + 1)
                    {
                        rendererOptions[j].setPosition(++i);
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    rendererOptions[j].setPosition(i);
                }
            }
        }
        if (!known) return false;
    }
    return true;
}

