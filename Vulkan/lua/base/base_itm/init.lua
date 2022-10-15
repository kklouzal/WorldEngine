local BASE = {}

BASE.Test = function()
	print("Test")
end

BASE.Initialize = function()
	print("Initialize")
end

BASE.StartPrimaryAction = function(RayTrace)
	print("Start Primary")
end

BASE.EndPrimaryAction = function()
	print("End Primary")
end

BASE.StartSecondaryAction = function(RayTrace)
	print("Start Secondary")
end

BASE.EndSecondaryAction = function()
	print("EndSecondary")
end

BASE.ReloadAction = function()
	print("Reload")
end

BASE.OnSelect = function()
	print("Select")
end

BASE.OnDeselect = function()
	print("Deselect")
end

BASE.OnTick = function()
	print("Think")
end

Item.RegisterBase(BASE)