#include "ShulkerRenderer.h"
#include <algorithm>
#include <cstdio>

extern ItemStack ShulkerInventory[SHULKER_CACHE_SIZE][27];

static constexpr float S = 18.f;
static constexpr float I = 16.f;
static constexpr float P = (S - I) / 2.f;

static bool texLoaded = false;
static mce::TexturePtr PanelTex;
static mce::TexturePtr SlotTex;

static HashedString flushStr("ui_flush");
static KNineSliceHelper nine(16.f, 16.f, 4.f, 4.f); //custom nineslice helper


//write directly to the underlying mem using reinterpret_cast + const_cast to set values so it becomes something like "t.fontSize = 1.0f"
static inline void setTMD(TextMeasureData& t)
{
    *reinterpret_cast<float*>(const_cast<float*>(&t.fontSize)) = 1.f;
    *reinterpret_cast<bool*>(const_cast<bool*>(&t.renderShadow)) = true;
}

// find tint by char code
static mce::Color getTint(char code)
{
    for (auto const& [_, info] : kShulkerColors)
        if (info.code == code)
            return info.tint;

    return mce::Color::WHITE();
}

void ShulkerRenderer::Render(MinecraftUIRenderContext* ctx, HoverRenderer* hr, int idx, char colorCode)
{
    if (!ctx || !hr)
        return;

    // load textures at once 
    if (!texLoaded)
    {
        PanelTex = ctx->getTexture(
            { Core::PathView("textures/ui/dialog_background_opaque"), ResourceFileSystem::UserPackage }, true);

        SlotTex = ctx->getTexture(
            { Core::PathView("textures/ui/item_cell"), ResourceFileSystem::UserPackage }, true);

        texLoaded = true;
    }

    // this is the tooltip at top left, <color>Shulker Box, probs changing it
    std::string tip = hr->mFilteredContent;

    float textH = ((int)std::count(tip.begin(), tip.end(), '\n') + 1) * 10.f;

    float px = hr->mCursorX + hr->mOffsetX;
    float py = hr->mCursorY + hr->mOffsetY;

    float pw = S * 9;
    float ph = S * 3 + textH;

    mce::Color panelTint = getTint(colorCode);

    // panel
    nine.draw(ctx, PanelTex.getClientTexture(), { px - 4.f, px + pw + 4.f, py - 4.f, py + ph + 4.f });

    //background panel can be tinted here
    ctx->flushImages(panelTint, 1.f, flushStr);

    // slot grid, worth noting that tinting the slots causes miscolored glints 
    const auto& slot = SlotTex.getClientTexture();

    for (int i = 0; i < 27; ++i)
    {
        int x = i % 9;
        int y = i / 9;

        glm::vec2 pos(px + S * x, py + textH + S * y);
        ctx->drawImage(slot, pos, { S, S }, { 0.f, 0.f }, { 1.f, 1.f }, false);
    }
    //imp, prevents the tint from bleeding into the ench glint
    ctx->flushImages(mce::Color::WHITE(), 1.f, flushStr);

    //ItemRenderer
   /*create a temp BARC on stack, LL exposes a $ctor so, allocate raw mem reinterpret that mem as BARC ptr
   manually invoke the internal $ctor to initilaize the obj this produces a valid rendercontext that itemrenderer needs
   in order to render items, the obj lives only for this func call,*/

    alignas(BaseActorRenderContext) unsigned char mem[sizeof(BaseActorRenderContext)];
    auto* barc = reinterpret_cast<BaseActorRenderContext*>(mem);
    barc->$ctor(ctx->mScreenContext, ctx->mClient, ctx->mClient.getMinecraftGame_DEPRECATED());

    auto& ir = barc->mItemRenderer;

    TextMeasureData tmd{};
    setTMD(tmd);
    CaretMeasureData cmd{};

    //items and glint
    for (int i = 0; i < 27; ++i)
    {
        int x = i % 9;
        int y = i / 9;

        ItemStack& s = ShulkerInventory[idx][i];
        if (s.isNull())
            continue;

        float sl = px + S * x;
        float st = py + textH + S * y;

        float ix = sl + P;
        float iy = st + P;

        ir.renderGuiItemNew(*barc, s, 0, ix, iy, false, 1.f, 1.f, 1.f, 0);

        if (s.mItem && s.mItem->isGlint(s))
            ir.renderGuiItemNew(*barc, s, 0, ix, iy, true, 1.f, 1.f, 1.f, 0);
    }

    ctx->flushImages(mce::Color::WHITE(), 1.f, flushStr);

    // durability and counts
    for (int i = 0; i < 27; ++i)
    {
        int x = i % 9;
        int y = i / 9;

        ItemStack& s = ShulkerInventory[idx][i];
        if (s.isNull())
            continue;

        float sl = px + S * x;
        float st = py + textH + S * y;

        //stack count
        if (s.mCount > 1)
        {
            char buf[8];
            snprintf(buf, 8, "%u", s.mCount);

            ctx->drawDebugText(
                { sl + 1.f, sl + S - 1.f, st + S - 11.f, st + S - 1.f },
                buf,
                mce::Color::WHITE(),
                1.f,
                (ui::TextAlignment)1,
                tmd,
                cmd
            );
        }

        //durability
        if (s.mItem && s.mItem->getMaxDamage() > 0)
        {
            int d = s.mItem->getDamageValue(s.mUserData.get());
            if (d > 0)
            {
                float dur = 1.f - (float)d / (float)s.mItem->getMaxDamage();

                float bx = sl + 2.f;
                float by = st + S - 4.f;
                float bw = S - 4.f;

                ctx->fillRectangle({ bx, bx + bw, by, by + 1.5f },
                                   mce::Color(0.f, 0.f, 0.f, 1.f), 1.f);

                ctx->fillRectangle({ bx, bx + bw * dur, by, by + 1.5f },
                                   mce::Color(1.f - dur, dur, 0.f, 1.f), 1.f);
            }
        }
    }
    // this is imp prevents the durability bar colors from bleeding into ench glint
    ctx->flushImages(mce::Color::WHITE(), 1.f, flushStr);

    //tooltip text <mfilteredcontent>
    ctx->drawDebugText(
        { px, px + pw, py, py + textH },
        std::move(tip),
        mce::Color::WHITE(),
        1.f,
        (ui::TextAlignment)0,
        tmd,
        cmd
    );

    ctx->flushText(0.f, std::optional<float>{});
}

ShulkerRenderer shulkerRenderer;
