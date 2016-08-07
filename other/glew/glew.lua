GLEW = {
	basepath = PathDir(ModuleFilename()),
	
	OptFind = function (name, required)	
		local check = function(option, settings)
			option.value = false
			option.use_glewconfig = false
			option.use_winlib = 0
			option.lib_path = nil
			
			if ExecuteSilent("glew-config") > 0 and ExecuteSilent("glew-config --cflags") == 0 then
				option.value = true
				option.use_glewconfig = true
			end
			
			if platform == "win32" then
				option.value = true
				option.use_winlib = 32
			elseif platform == "win64" then
				option.value = true
				option.use_winlib = 64
--[[	TODO: bundle the libs for these configurations aswell when needed:
			elseif platform == "macosx" and string.find(settings.config_name, "32") then
				option.value = true
			elseif platform == "macosx" and string.find(settings.config_name, "64") then
				option.value = true
			elseif platform == "linux" and arch == "ia32" then
				option.value = true
]]			elseif platform == "linux" and arch == "amd64" then
				option.value = true
			end
		end
		
		local apply = function(option, settings)
			-- include path
			settings.cc.includes:Add(GLEW.basepath .. "/include")
			
			if family == "windows" then
				settings.link.libpath:Add(GLEW.basepath .. "/windows/lib" .. option.use_winlib)
				settings.link.libs:Add("glew32")
			elseif platform == "linux" then
				if arch == "amd64" then
					settings.link.libpath:Add(GLEW.basepath .. "/linux/lib64")
				else
					settings.link.libpath:Add(GLEW.basepath .. "/linux/lib32")
				end
				settings.link.libs:Add("GLEW")
			end
		end
		
		local save = function(option, output)
			output:option(option, "value")
			output:option(option, "use_winlib")
		end
		
		local display = function(option)
			if option.value == true then
				if option.use_glewconfig == true then return "using glew-config" end
				if option.use_winlib == 32 then return "using supplied win32 libraries" end
				if option.use_winlib == 64 then return "using supplied win64 libraries" end
				return "using bundled libs"
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
