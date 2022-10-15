local BASE = {}

BASE.__tostring = function()
	return "Base"
end

function BASE:Test()
	print("Base:Test()")
end

function BASE:Initialize()

end

Ply.RegisterBase(BASE)