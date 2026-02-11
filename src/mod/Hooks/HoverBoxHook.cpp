#include <Windows.h>
#include "UI/ShulkerRenderer.h"
#include "Util/KHoverRenderer.h"
#include <libhat.hpp>
#include <safetyhook.hpp>
#include <ll/api/io/LoggerRegistry.h>

struct MinecraftUIRenderContext;
struct IClientInstance;
struct RectangleArea;

using RenderHoverBoxFn = void(__fastcall*)(HoverRenderer*, MinecraftUIRenderContext*, IClientInstance*, RectangleArea*, float);

static SafetyHookInline HoverHook{};
static RenderHoverBoxFn RenderHoverBoxFnOriginal = nullptr;
static auto logger = ll::io::LoggerRegistry::getInstance().getOrCreate("ShulkerPreview");

// convert hexchar to int
static inline int hexv(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static void __fastcall RenderHoverBoxFnHook(
    HoverRenderer* self,
    MinecraftUIRenderContext* ctx,
    IClientInstance* client,
    RectangleArea* aabb,
    float f)
{
    if (!self || !ctx)
    {
        logger->warn("Invalid ctx/self in hover hook");
        RenderHoverBoxFnOriginal(self, ctx, client, aabb, f);
        return;
    }

    // this bit omits whatever we're hovering over
    const std::string& text = self->mFilteredContent;

    // detect encoded prev marker, look ShulkerHook for more inf
    bool encoded =
        text.size() >= 9 &&
        text[0] == '\xC2' && text[1] == '\xA7' &&
        text[3] == '\xC2' && text[4] == '\xA7' &&
        text[6] == '\xC2' && text[7] == '\xA7';

    if (!encoded)
    {
        RenderHoverBoxFnOriginal(self, ctx, client, aabb, f);
        return;
    }

    // decode shulker cache index
    int index = ((hexv(text[2]) << 4) | hexv(text[5])) & (SHULKER_CACHE_SIZE - 1);

    if (index < 0 || index >= SHULKER_CACHE_SIZE)
    {
        logger->warn("Invalid shulker cache index: {}", index);
        return;
    }

    // our custom hoverbox, you can call both vanilla and custom, why??
    // well we can keep vanilla hoverbox and below it we can trigger our custom
    // via a keybind

    // RenderHoverBoxFnOriginal(self, ctx, client, aabb, f);

    shulkerRenderer.Render(ctx, self, index, text[8]);
}

// sigscan & hook install
void InstallHoverRendererHook()
{
    auto signature = hat::parse_signature(
        "48 8B C4 48 89 58 08 48 89 70 10 57 48 81 EC 60");

    auto module = hat::process::get_process_module();
    auto span   = hat::process::get_module_data(module);

    auto result = hat::find_pattern(span.begin(), span.end(), signature.value());
    if (!result.has_result())
    {
        logger->error("HoverRenderer signature not found");
        return;
    }

    HoverHook = safetyhook::create_inline(result.get(), RenderHoverBoxFnHook);
    RenderHoverBoxFnOriginal = HoverHook.original<RenderHoverBoxFn>();

    logger->info("HoverRenderer hook installed");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        InstallHoverRendererHook();
    }
    return TRUE;
}
