local ITEM = {}

function ITEM:Initialize()
	print("Initialize")
end

function ITEM:StartPrimaryAction()
	print("Start Primary")
	local Owner = self:GetOwner()
	local pos = Owner:GetFirePos()
	local ang = Owner:GetFireAng()
	local to = pos + ang * Vector3(1000, 1000, 1000)
	
	local res = Util.TraceLine(pos, to)
	print ("Trace Result Hit: "..tostring(res.HasHit))
	if (res.HasHit) then
		local hitEnt = res.HitEntity
		self.TargetDistance = pos:Distance(hitEnt:GetPos())
		print("Hit Entity: "..tostring(hitEnt).." Distance: "..tostring(self.TargetDistance))
		self.SelectedEntity = hitEnt
	end
end

function ITEM:EndPrimaryAction()
	print("End Primary")
	self.SelectedEntity = nil
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
	if (self.SelectedEntity ~= nil) then
		local Owner = self:GetOwner()
		local pos = Owner:GetFirePos()
		local ang = Owner:GetFireAng()
		
		local Dist = self.TargetDistance
		local DestinationPos = pos + ang * Vector3(Dist,Dist,Dist)
		self.SelectedEntity:SetPos(DestinationPos)
	end
end

Item.Register("itm_default", ITEM)