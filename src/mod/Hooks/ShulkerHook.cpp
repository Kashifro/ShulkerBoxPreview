#include "ShulkerHook.h"
#include "Util/KColor.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/item/ShulkerBoxBlockItem.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/level/Level.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/IntTag.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/ByteTag.h"
#include "mc/nbt/ShortTag.h"
#include "mc/safety/RedactableString.h"

// circular cache stores decoded shulker invs each tooltip instance refs an index
// into this cache which the custom renderer later uses to draw preview contents
ItemStack ShulkerInventory[SHULKER_CACHE_SIZE][27];
int ShulkerIndex = 0;

/*
// for ref
// reads slot from byte/short/int nbt forms
static inline int readSlotIndex(const CompoundTag& tag)
{
    if (!tag.contains("Slot"))
        return -1;

    const Tag& t = tag.at("Slot");

    switch (t.getId())
    {
        case Tag::Type::Byte:  return (int)t.get<ByteTag>().data;
        case Tag::Type::Short: return (int)t.get<ShortTag>().data;
        case Tag::Type::Int:   return t.get<IntTag>().data;
        default:               return -1;
    }
}
*/

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
    const int thisIndex = ShulkerIndex;

    char colorCode = '0';
    if (auto it = kShulkerColors.find(stack.getRawNameId()); it != kShulkerColors.end())
        colorCode = it->second.code;

    std::string& s = text.mUnredactedString;

    // keep only first line we draw custom contents below this
    if (auto nl = s.find('\n'); nl != std::string::npos)
        s.erase(nl);

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
    if (!tag || !tag->contains("Items", Tag::Type::List))
        return;

    const ListTag& items = tag->at("Items").get<ListTag>();

    for (size_t i = 0; i < items.size(); ++i)
    {
        const Tag* entry = items.get((int)i);
        if (!entry || entry->getId() != Tag::Type::Compound)
            continue;

        const CompoundTag* itemTag = static_cast<const CompoundTag*>(entry);
        if (!itemTag || !itemTag->contains("Slot", Tag::Type::Byte))
            continue;

        // slot is known to be stored as ByteTag in shulker item nbt
        uint8_t slot = (uint8_t)itemTag->at("Slot").get<ByteTag>().data;
        if (slot >= 27)
            continue;

        // load directly from item nbt
        ItemStack parsed;
        parsed._loadItem(*itemTag);

        ShulkerInventory[thisIndex][slot] = parsed;
    }
}
