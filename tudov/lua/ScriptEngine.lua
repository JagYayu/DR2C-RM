local lockedMetatables = setmetatable({}, { __mode = "k" })
local enumTables

local type = type

return {
	initialize = function()
		-- jit.off()
	end,
	markAsLocked = function(t)
		if type(t) == "table" then
			lockedMetatables[t] = true
		end
	end,
	postProcessSandboxing = function(sandbox, globals)
		local getmetatable = globals.getmetatable
		local setmetatable = globals.setmetatable

		function sandbox.getmetatable(object)
			if type(object) ~= "table" then
				error(("bad argument #1 to 'object' (table expected, got %s)"):format(type(object)), 2)
			end

			local mt = getmetatable(object)
			if mt ~= nil and lockedMetatables[mt] then
				error("metatable is locked, inaccessible in sandboxed environment", 2)
			end

			return mt
		end

		function sandbox.setmetatable(table, metatable)
			if type(table) ~= "table" then
				error(("bad argument #1 to 'table' (table expected, got %s)"):format(type(table)), 2)
			elseif type(metatable) ~= "table" then
				error(("bad argument #2 to 'metatable' (table expected, got %s)"):format(type(metatable)), 2)
			end

			local mt = getmetatable(table)
			if mt ~= nil and lockedMetatables[mt] then
				error("metatable is locked, unmodifiable in sandboxed environment", 2)
			end

			return setmetatable(table, metatable)
		end
	end,
}
