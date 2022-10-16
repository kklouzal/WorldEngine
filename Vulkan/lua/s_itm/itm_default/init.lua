local ITEM = {}

function ITEM:Initialize()
	print("Initialize")
end

function ITEM:StartPrimaryAction()
	print("Start Primary")
	local Owner = self:GetOwner()
	local pos = Owner:GetFirePos()
	local ang = Owner:GetFireAng()
	print("pos "..tostring(pos))
	print("ang "..tostring(ang))
	--	pos + ang * dist
	local to = pos + ang * Vector3(1000, 1000, 1000) 
	print("to  "..tostring(to))
	
	local res = Util.TraceLine(pos, to)
	print ("Trace Result Hit: "..tostring(res.HasHit))
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