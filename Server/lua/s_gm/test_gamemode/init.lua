local GAMEMODE = {}

GAMEMODE.OnPlayerSpawn = function(in_plyid, in_ply)
	if (in_ply == nil) then
		print("Player is nil")
	else
		in_ply:Give("itm_default")
	end
end

GM.Register(GAMEMODE)