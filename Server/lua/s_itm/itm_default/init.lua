local ITEM = {}

function ITEM:Test()
	print("Test")
end

function ITEM:Initialize()

end

Item.Register("itm_default", ITEM)