#include "KNineSliceHelper.h"

KNineSliceHelper::KNineSliceHelper(
    float texW, float texH,
    float sliceW, float sliceH)
{
    mTexW = texW;
    mTexH = texH;
    mSliceW = sliceW;
    mSliceH = sliceH;

    float midW = texW - sliceW*2.f;
    float midH = texH - sliceH*2.f;

    auto make = [&](float x,float y,float w,float h)
    {
        return Slice{
            {x/texW,y/texH},
            {w/texW,h/texH},
            w,h
        };
    };

    mSlices[0]=make(0,0,sliceW,sliceH);
    mSlices[1]=make(sliceW,0,midW,sliceH);
    mSlices[2]=make(texW-sliceW,0,sliceW,sliceH);

    mSlices[3]=make(0,sliceH,sliceW,midH);
    mSlices[4]=make(sliceW,sliceH,midW,midH);
    mSlices[5]=make(texW-sliceW,sliceH,sliceW,midH);

    mSlices[6]=make(0,texH-sliceH,sliceW,sliceH);
    mSlices[7]=make(sliceW,texH-sliceH,midW,sliceH);
    mSlices[8]=make(texW-sliceW,texH-sliceH,sliceW,sliceH);
}

void KNineSliceHelper::draw(
    MinecraftUIRenderContext* ctx,
    const mce::ClientTexture& tex,
    const RectangleArea& rect)
{
    float x = rect._x0;
    float y = rect._y0;
    float w = rect._x1 - rect._x0;
    float h = rect._y1 - rect._y0;

    float midW = w - mSliceW*2.f;
    float midH = h - mSliceH*2.f;

    glm::vec2 pos[9]={
        {x,y},
        {x+mSliceW,y},
        {x+w-mSliceW,y},
        {x,y+mSliceH},
        {x+mSliceW,y+mSliceH},
        {x+w-mSliceW,y+mSliceH},
        {x,y+h-mSliceH},
        {x+mSliceW,y+h-mSliceH},
        {x+w-mSliceW,y+h-mSliceH}
    };

    glm::vec2 size[9]={
        {mSliceW,mSliceH},
        {midW,mSliceH},
        {mSliceW,mSliceH},
        {mSliceW,midH},
        {midW,midH},
        {mSliceW,midH},
        {mSliceW,mSliceH},
        {midW,mSliceH},
        {mSliceW,mSliceH}
    };

    for(int i=0;i<9;i++)
    {
        ctx->drawImage(
            tex,
            pos[i],
            size[i],
            mSlices[i].uv,
            mSlices[i].uvSize,
            false);
    }
}
