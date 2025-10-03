#pragma once

struct Cheats
{
	static void UpdateGodMode();
	static void UpdateInfiniteStamina();
};

struct CVarsStruct
{
	bool GodMode = false;
	bool Stamina = false;
} inline CVars;