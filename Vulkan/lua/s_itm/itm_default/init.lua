local ITEM = {}

function ITEM:Initialize()
	print("Initialize")
end

function ITEM:StartPrimaryAction()
	print("Start Primary")
	local pos = self:GetPos()
	print("pos "..tostring(pos))
end

function ITEM:EndPrimaryAction()
	print("End Primary")
end

function ITEM:StartSecondaryAction()
	print("Start Secondary")
end

function ITEM:EndSecondaryAction()
	print("EndSecondary")
end

function ITEM:ReloadAction()
	print("Reload")
end

function ITEM:OnSelect()
	print("Select")
end

function ITEM:OnDeselect()
	print("Deselect")
end

function ITEM:OnTick()
	print("Think")
end

Item.Register("itm_default", ITEM)