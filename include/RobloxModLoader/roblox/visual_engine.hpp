#pragma once
#include "adorn_render.hpp"
#include "RBXG3D/Device.h"

class VertexStreamer;

class VisualEngine {
private:
    char pad_0000[0x84];

public:
    Device *device;

private:
    char pad_0008[0x8];

public:
    char renderCameras[0x60];

private:
    char pad_00F0[0x638];

public:
    AdornRender *adornRender;

private:
    char pad_0730[0x10];

public:
    VertexStreamer *vertexStreamer;

private:
    char pad_0744[0x4];

public:
    float width;
    float height;
};
