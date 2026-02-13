#pragma once

#include "mc/world/item/ItemStack.h"
#include "mc/client/renderer/screen/MinecraftUIRenderContext.h"
#include "mc/deps/input/RectangleArea.h"
#include "mc/deps/core/math/Color.h"
#include "mc/client/gui/TextAlignment.h"
#include "mc/client/gui/TextMeasureData.h"
#include "mc/client/gui/CaretMeasureData.h"
#include "mc/client/renderer/BaseActorRenderContext.h"
#include "mc/client/renderer/actor/ItemRenderer.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/client/game/IClientInstance.h"
#include "mc/deps/core/resource/ResourceLocation.h"
#include "mc/deps/core/file/PathView.h"
#include "mc/client/gui/controls/MeasureResult.h"
#include "mc/deps/input/Keyboard.h"
#include "Util/KColor.h"
#include "Util/KHoverRenderer.h"
#include "Util/KNineSliceHelper.h"
#include "Hooks/ShulkerHook.h"
#include "mc/world/item/Item.h"

class MinecraftUIRenderContext;
class Font;

class ShulkerRenderer
{
    
public:
    void Render(MinecraftUIRenderContext* ctx, HoverRenderer* hoverRenderer, int index, char colorCode);
};

extern ShulkerRenderer shulkerRenderer;
