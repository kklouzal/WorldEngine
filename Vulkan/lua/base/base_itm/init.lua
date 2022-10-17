local BASE = {}

function BASE:Initialize()
	print("Initialize - Base")
end

function BASE:StartPrimaryAction()
	print("Start Primary - Base")
end

function BASE:EndPrimaryAction()
	print("End Primary - Base")
end

function BASE:StartSecondaryAction()
	print("Start Secondary - Base")
end

function BASE:EndSecondaryAction()
	print("EndSecondary - Base")
end

function BASE:ReloadAction()
	print("Reload - Base")
end

function BASE:OnSelect()
	print("Select - Base")
end

function BASE:OnDeselect()
	print("Deselect - Base")
end

function BASE:OnTick()
	print("Think - Base")
end

Item.RegisterBase(BASE)