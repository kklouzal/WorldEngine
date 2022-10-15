local ITEM = {}

ITEM.Test = function()
	print("Test")
end

ITEM.Initialize = function()
	print("Initialize")
end

ITEM.StartPrimaryAction = function(RayTrace)
	print("Start Primary")
end

ITEM.EndPrimaryAction = function()
	print("End Primary")
end

ITEM.StartSecondaryAction = function(RayTrace)
	print("Start Secondary")
end

ITEM.EndSecondaryAction = function()
	print("EndSecondary")
end

ITEM.ReloadAction = function()
	print("Reload")
end

ITEM.OnSelect = function()
	print("Select")
end

ITEM.OnDeselect = function()
	print("Deselect")
end

ITEM.OnTick = function()
	print("Think")
end

Item.Register("itm_default", ITEM)