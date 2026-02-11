#pragma once
#include <string>
#include <unordered_map>
#include "mc/deps/core/math/Color.h"

struct ShulkerColorInfo {
    char code;
    const char* name;
    mce::Color tint;
};

inline const std::unordered_map<std::string, ShulkerColorInfo> kShulkerColors = {
    {"undyed_shulker_box", {'0',"Undyed",      {0.45f,0.42f,0.40f,1.f}}},
    {"white_shulker_box",  {'1',"White",       {0.78f,0.76f,0.74f,1.f}}},
    {"light_gray_shulker_box", {'2',"Light Gray",{0.55f,0.53f,0.52f,1.f}}},
    {"gray_shulker_box",   {'3',"Gray",        {0.32f,0.31f,0.30f,1.f}}},
    {"black_shulker_box",  {'4',"Black",       {0.06f,0.05f,0.05f,1.f}}},
    {"brown_shulker_box",  {'5',"Brown",       {0.33f,0.25f,0.14f,1.f}}},
    {"red_shulker_box",    {'6',"Red",         {0.55f,0.20f,0.18f,1.f}}},
    {"orange_shulker_box", {'7',"Orange",      {0.70f,0.42f,0.18f,1.f}}},
    {"yellow_shulker_box", {'8',"Yellow",      {0.78f,0.72f,0.22f,1.f}}},
    {"lime_shulker_box",   {'9',"Lime",        {0.42f,0.65f,0.22f,1.f}}},
    {"green_shulker_box",  {'a',"Green",       {0.18f,0.40f,0.18f,1.f}}},
    {"cyan_shulker_box",   {'b',"Cyan",        {0.18f,0.55f,0.55f,1.f}}},
    {"light_blue_shulker_box", {'c',"Light Blue",{0.28f,0.46f,0.62f,1.f}}},
    {"blue_shulker_box",   {'d',"Blue",        {0.18f,0.24f,0.58f,1.f}}},
    {"purple_shulker_box", {'e',"Purple",      {0.45f,0.26f,0.60f,1.f}}},
    {"magenta_shulker_box",{'f',"Magenta",     {0.65f,0.34f,0.58f,1.f}}},
    {"pink_shulker_box",   {'g',"Pink",        {0.78f,0.52f,0.62f,1.f}}},
};
