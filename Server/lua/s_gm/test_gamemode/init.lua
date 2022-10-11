local GAMEMODE = {}

GAMEMODE.OnPlayerSpawn = function(in_plyid, in_ply)
	print(in_plyid)	--prints nil	<--- wtf m8?!
	print(in_ply)	--prints tableID
	if (in_ply == nil) then
		print("Player is nil")
	else	--	executes this branch
		in_ply:Test()	--	prints correct output
	end
	Ply.Objects[in_plyid]:Test()	--	error indexing nil value
	return true
end

GM.Register(GAMEMODE)

function GlobalTest(in_plyid, in_ply)
	print(in_plyid)
	print(in_ply)
end