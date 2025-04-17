#pragma once
#include "adorn_render.hpp"
#include "RBXG3D/Device.h"

class VertexStreamer;

class VisualEngine {
private:
    char pad_0000[0xA8];

public:
    Device *device;

private:
    char pad_00B0[0x670];

public:
    uintptr_t* dataModel;
    AdornRender *adornRender;
    VertexStreamer *vertexStreamer;

private:
    char pad_0744[0x8];

public:
    float width;
    float height;
};
