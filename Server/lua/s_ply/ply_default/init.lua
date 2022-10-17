local PLAYER = {}

PLAYER.__index = PLAYER

PLAYER.__tostring = function()
	return "Player"
end

function PLAYER:Test()
	print("Test")
end

function PLAYER:Initialize()

end

Ply.Register("ply_default", PLAYER)