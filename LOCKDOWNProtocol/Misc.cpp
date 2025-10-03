#include "Cheats.h"
#include "Utils.h"

void Cheats::UpdateGodMode()
{
	if (GVars.MecChar && CVars.GodMode)
		GVars.MecChar->Health = 100;
}

void Cheats::UpdateInfiniteStamina()
{
	if (GVars.MecChar && CVars.Stamina)
		GVars.MecChar->Stamina = 0;
}