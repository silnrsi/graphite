#pragma once

#include "RenderedLine.h"


class Renderer
{
public:
    virtual ~Renderer() {}
    virtual void renderText(const char * utf8, size_t length, RenderedLine * result) = 0;
};
