#pragma once
#include "mc/client/gui/controls/ImageInfo.h"
#include "mc/client/gui/controls/NinesliceInfo.h"
#include "mc/client/renderer/screen/MinecraftUIRenderContext.h"

class KNineSliceHelper
{
public:
    KNineSliceHelper(float texW, float texH, float sliceW, float sliceH);

    void draw( MinecraftUIRenderContext* ctx, const mce::ClientTexture& tex, const RectangleArea& rect);

private:
    struct Slice {
        glm::vec2 uv;
        glm::vec2 uvSize;
        float pxW;
        float pxH;
    };

    Slice mSlices[9];

    float mTexW;
    float mTexH;
    float mSliceW;
    float mSliceH;
};
