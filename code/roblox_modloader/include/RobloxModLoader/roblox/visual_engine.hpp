#pragma once
#include "adorn_render.hpp"
#include "RBXG3D/Device.h"

class VertexStreamer;

class VisualEngine {
private:
    char pad_0000[0x90];

public:
    Device *device;

private:
    char pad_00B0[0x668];

public:
    uintptr_t* dataModel;
    AdornRender *adornRender;
    VertexStreamer *vertexStreamer;

private:
    char pad_0744[0x8];

public:
    float width;
    float height;

private:
	std::byte pad_0750[0x10];

public:
	class SceneUpdater* sceneUpdater;

private:
    std::byte pad_0761[0x10];

public:
	class TextureCompositor* textureCompositor;
	class TextureManager* textureManager;
   
private:
	std::byte pad_0768[0x18];

public:
    class ShaderManager* shaderManager;

private:
	std::byte pad_0780[0x40];

public:
	class UITextureRenderer* uiTextureRenderer;

private:
	std::byte pad_07C0[0x40];

public:
	class RenderView* renderView;
	class MeshContentProvider* meshContentProvider;
	class ContentProvider* contentProvider;
	class Lighting* lighting;
	
private:
	std::byte pad_0800[0x50];

public:
	class LightGridUnified* lightGridUnified;

private:
	std::byte pad_0850[0x18];

public:
	class TextureAtlas* textureAtlas;

private:
	std::byte pad_0870[0x20];

public:
	class ShadowMapSystem* shadowMapSystem;
};
