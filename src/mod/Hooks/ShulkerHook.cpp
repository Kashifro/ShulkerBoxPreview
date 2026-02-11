#include "ShulkerHook.h"
#include "Util/KColor.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/item/ShulkerBoxBlockItem.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/level/Level.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/ByteTag.h"
#include "mc/safety/RedactableString.h"

using namespace ll;

// circular cache stores decoded shulker invs each tooltip instance refs an index
// into this cache which the custom renderer later uses to draw preview contents
ItemStack ShulkerInventory[SHULKER_CACHE_SIZE][27];
int ShulkerIndex = 0;

LL_AUTO_TYPE_INSTANCE_HOOK(
    ShulkerTooltipHook,
    HookPriority::Normal,
    ShulkerBoxBlockItem,
    &ShulkerBoxBlockItem::$appendFormattedHovertext,
    void,
    ItemStackBase const& stack,
    Level& level,
    Bedrock::Safety::RedactableString& text,
    bool showCategory)
{
    // call original first
    origin(stack, level, text, showCategory);

    ShulkerIndex = (ShulkerIndex + 1) % SHULKER_CACHE_SIZE;
    int thisIndex = ShulkerIndex;

    char colorCode = '0';
    auto it = kShulkerColors.find(stack.getRawNameId()); // probs changing this
    if (it != kShulkerColors.end())
        colorCode = it->second.code;

    std::string& s = text.mUnredactedString;

    // remove vanilla "Items" tooltip section entirely preventing duplicate listing
    // contained items since we will render them visually instead.
    if (auto pos = s.find("Items"); pos != std::string::npos)
        s.erase(pos);
    if (auto pos = s.find('\n'); pos != std::string::npos)
        s.erase(pos);

    // encode cache index in hex, color encodes shulker tint
    // HoverBoxHook later detects this sequence and replaces vanilla hoverbox with custom
    static const char hex[] = "0123456789abcdef";

    std::string prefix;
    prefix += "\xC2\xA7";
    prefix += hex[(thisIndex >> 4) & 0xF];
    prefix += "\xC2\xA7";
    prefix += hex[thisIndex & 0xF];
    prefix += "\xC2\xA7";
    prefix += colorCode;

    // prepend encoded metadata to tooltip string
    s.insert(0, prefix);

    // clear cached slots
    for (auto& slot : ShulkerInventory[thisIndex])
        slot = ItemStack();

    const CompoundTag* tag = stack.mUserData.get();
    if (!tag || !tag->contains("Items"))
        return;

    const ListTag& items = tag->at("Items").get<ListTag>();

    for (size_t i = 0; i < items.size(); ++i)
    {
        const CompoundTag* itemTag =
            static_cast<const CompoundTag*>(items.get((int)i));
        if (!itemTag)
            continue;

        uint8_t slot =
            (uint8_t)itemTag->at("Slot").get<ByteTag>().data;

        if (slot < 27)
            ShulkerInventory[thisIndex][slot]._loadItem(*itemTag);
    }
}
