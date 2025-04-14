#pragma once
#include "visual_engine.hpp"

class Device;

class RenderView {
private:
    char pad_0000[0x4];
public:
    Device* Device;
    VisualEngine* VisualEngine;
};