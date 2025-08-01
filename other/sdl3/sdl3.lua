SDL3 = {
    basepath = PathDir(ModuleFilename()),

    OptFind = function (name, required)
        local check = function(option, settings)
            option.value = false
            option.use_pkgconfig = false
            option.use_winlib = 0
            option.lib_path = nil
            
            if platform == "win32" then
                option.value = true
                option.use_winlib = 32
            elseif platform == "win64" then
                option.value = true
                option.use_winlib = 64
            elseif ExecuteSilent("pkg-config") > 0  and ExecuteSilent("pkg-config sdl3") == 0 then
                option.value = true
                option.use_pkgconfig = true
            end
        end
        
        local apply = function(option, settings)
            if option.use_pkgconfig == true then
                settings.cc.flags:Add("`pkg-config --cflags sdl3`")
                settings.link.flags:Add("`pkg-config --libs sdl3`")
            elseif option.use_winlib > 0 then
                settings.cc.includes:Add(SDL3.basepath .. "/include")
                if option.use_winlib == 32 then
                    settings.link.libpath:Add(SDL3.basepath .. "/windows/lib32")
                else
                    settings.link.libpath:Add(SDL3.basepath .. "/windows/lib64")
                end
                settings.link.libs:Add("SDL3")
            end
        end
        
        local save = function(option, output)
            output:option(option, "value")
            output:option(option, "use_pkgconfig")
            output:option(option, "use_winlib")
        end
        
        local display = function(option)
            if option.value == true then
                if option.use_pkgconfig == true then return "using pkg-config (SDL3)" end
                if option.use_winlib == 32 then return "using supplied win32 libraries (SDL3)" end
                if option.use_winlib == 64 then return "using supplied win64 libraries (SDL3)" end
                return "using unknown method"
            else
                if option.required then
                    return "not found (required)"
                else
                    return "not found (optional)"
                end
            end
        end
        
        local o = MakeOption(name, 0, check, save, display)
        o.Apply = apply
        o.include_path = nil
        o.lib_path = nil
        o.required = required
        return o
    end
}