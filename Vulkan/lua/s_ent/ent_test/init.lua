print("Hello From Lua Test")

local ENT = {}

function ENT:Test2()
	print("Ent:Test2()")
end

Ents.Register("ent_test", ENT)

--test inheritance
Ents.ObjectMetatables["ent_test"]:Test2()	--prints 'ENT:Test2()'
Ents.ObjectMetatables["ent_test"]:Test()	--prints 'Base:Test()'

--overwrite base function
function ENT:Test()
	print("Ent:Test()")
end
Ents.ObjectMetatables["ent_test"]:Test()	--prints 'ENT:Test()'