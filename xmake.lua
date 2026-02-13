add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

option("target_type")
    set_default("client")
    set_showmenu(true)
    set_values("server", "client")
option_end()

add_requires("levilamina", {configs = {target_type = get_config("target_type")}})
add_requires("levibuildscript")
add_requires("libhat")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("ShulkerBoxPreview")
    set_kind("shared")
    set_languages("c++23")
    set_symbols("debug")
    set_exceptions("none")

    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")

    add_cxflags("/EHa", "/utf-8", "/W4",
        "/w44265", "/w44289", "/w44296",
        "/w45263", "/w44738", "/w45204")

    add_defines("NOMINMAX", "UNICODE", "ZYCORE_STATIC_BUILD","ZYDIS_STATIC_BUILD")

    add_packages("levilamina")
    add_packages("libhat")

    add_includedirs("external/zycore-c/include", {public = true})
    add_files("external/zycore-c/src/**.c")
    
    add_includedirs("external/zydis/include", {public = true})
    add_includedirs("external/zydis/include/Zydis", {public = true})
    add_includedirs("external/zydis/src", {public = true})
    add_files("external/zydis/src/**.c")

    add_includedirs("external/safetyhook/include", {public = true})
    add_files("external/safetyhook/src/**.cpp")

    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_includedirs("src/mod/")
