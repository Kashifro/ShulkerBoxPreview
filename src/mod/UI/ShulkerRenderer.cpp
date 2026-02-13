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

// direct keyboard poll, keep this simple no settings screen/key registry path
static inline bool isShowContentsHeld()
{
    auto& keyStates = Keyboard::_states();
    return keyStates[Keyboard::Lshift] != 0;
}

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

// sentinel reset, draw a tiny offscreen img then flush white so tint state actually resets
static inline void resetImageTintWithSentinel(MinecraftUIRenderContext* ctx, const mce::ClientTexture& sentinelTex)
{
    // if batch is empty white flush can be ignored by pipeline, this forces a real flush
    ctx->drawImage(sentinelTex, { -10000.f, -10000.f }, { 1.f, 1.f }, { 0.f, 0.f }, { 1.f, 1.f }, false);
    ctx->flushImages(mce::Color::WHITE(), 1.f, flushStr);
}

// strip mc formatting bytes (\xC2\xA7 + code) so width calc matches what actually renders
static std::string stripFormattingCodesForMeasure(std::string const& line)
{
    std::string clean;
    clean.reserve(line.size());

    for (size_t i = 0; i < line.size();)
    {
        if (i + 2 < line.size() &&
            static_cast<unsigned char>(line[i]) == 0xC2 &&
            static_cast<unsigned char>(line[i + 1]) == 0xA7)
        {
            i += 3;
            continue;
        }

        clean.push_back(line[i]);
        ++i;
    }

    return clean;
}

// measure max line width from current debug font handle so panel can hug text in empty mode
static float measureTipWidth(MinecraftUIRenderContext* ctx, std::string const& text, TextMeasureData const& tmd, CaretMeasureData const& cmd)
{
    if (!ctx || text.empty())
        return 24.f;

    FontHandle const& debugFont = ctx->mDebugTextFontHandle.get();
    Bedrock::NonOwnerPointer<FontHandle const> fontPtr(debugFont);
    if (!fontPtr)
        return 24.f;

    Bedrock::NotNullNonOwnerPtr<FontHandle const> fontRef(fontPtr);

    float maxW = 0.f;
    size_t start = 0;

    while (true)
    {
        size_t end = text.find('\n', start);
        std::string line =
            (end == std::string::npos) ? text.substr(start) : text.substr(start, end - start);

        std::string cleanLine = stripFormattingCodesForMeasure(line);
        MeasureResult mr = ctx->getMeasureStrategy().measureTextWidth(fontRef, cleanLine, tmd, cmd);
        glm::vec2 size = mr.mSize;
        maxW = std::max(maxW, size.x);

        if (end == std::string::npos)
            break;

        start = end + 1;
    }

    return std::max(maxW, 24.f);
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

    bool hasItems = false;
    for (int i = 0; i < 27; ++i)
    {
        if (!ShulkerInventory[idx][i].isNull())
        {
            hasItems = true;
            break;
        }
    }

    TextMeasureData tmd{};
    setTMD(tmd);
    CaretMeasureData cmd{};

    bool showContents = hasItems && isShowContentsHeld();

    if (!hasItems)
        tip += "\n\xC2\xA7" "7Empty";
    else if (!showContents)
        tip += "\n\xC2\xA7" "7L Shift : to show contents";

    float textH = ((int)std::count(tip.begin(), tip.end(), '\n') + 1) * 10.f;
    float textW = showContents ? (S * 9.f) : measureTipWidth(ctx, tip, tmd, cmd);

    float px = hr->mCursorX + hr->mOffsetX + 15.f;
    float py = hr->mCursorY + hr->mOffsetY;

    float pw = textW;
    float ph = showContents ? (S * 3 + textH) : textH;

    mce::Color panelTint = getTint(colorCode);

    // panel
    nine.draw(ctx, PanelTex.getClientTexture(), { px - 4.f, px + pw + 4.f, py - 4.f, py + ph + 4.f });

    //background panel can be tinted here
    ctx->flushImages(panelTint, 1.f, flushStr);

    if (showContents)
    {
        // draw grid/items only while key is held, hidden mode stays text-only tinted
        const auto& slot = SlotTex.getClientTexture();

        // slot grid stays tinted
        for (int i = 0; i < 27; ++i)
        {
            int x = i % 9;
            int y = i / 9;

            glm::vec2 pos(px + S * x, py + textH + S * y);
            ctx->drawImage(slot, pos, { S, S }, { 0.f, 0.f }, { 1.f, 1.f }, false);
        }
        // flush slot batch with tint, then hard reset so items/glint dont inherit slot color
        ctx->flushImages(panelTint, 1.f, flushStr);
        resetImageTintWithSentinel(ctx, slot);

        //ItemRenderer
       /*create a temp BARC on stack, LL exposes a $ctor so, allocate raw mem reinterpret that mem as BARC ptr
       manually invoke the internal $ctor to initilaize the obj this produces a valid rendercontext that itemrenderer needs
       in order to render items, the obj lives only for this func call,*/

        alignas(BaseActorRenderContext) unsigned char mem[sizeof(BaseActorRenderContext)];
        auto* barc = reinterpret_cast<BaseActorRenderContext*>(mem);
        barc->$ctor(ctx->mScreenContext, ctx->mClient, ctx->mClient.getMinecraftGame_DEPRECATED());

        auto& ir = barc->mItemRenderer;

        // pass 1, base item layer only, no glint yet
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
        }

        // imp reset again right before glint pass
        resetImageTintWithSentinel(ctx, slot);

        // pass 2, enchant glint overlay only
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

            if (s.isGlint() || s.isEnchanted())
                ir.renderGuiItemNew(*barc, s, 0, ix, iy, true, 1.f, 1.f, 1.f, 0);
        }

        // isolate next overlays so glint state doesnt bleed into count/dura/text
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
            if (s.isDamageableItem())
            {
                int maxD = (int)s.getMaxDamage();
                int d    = (int)s.getDamageValue();

                if (maxD > 0 && d > 0)
                {
                    float dur = 1.f - (float)d / (float)maxD;

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
