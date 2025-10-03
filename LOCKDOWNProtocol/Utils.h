#pragma once

#include <imgui.h>

#include "SDK/Engine_classes.hpp"
#include "SDK/Mec_classes.hpp"
#include "SDK/PC_classes.hpp"

using namespace SDK;

struct Utils
{
	static UWorld* GetWorldSafe(); // can return nullptr
	static APlayerController* GetPlayerController(); // can return nullptr
	static APC_C* GetAPC(); // can return nullptr
	static unsigned ConvertImVec4toU32(ImVec4 Color);
	static void PrintActors(const char* Exclude);
};

struct Variables
{
	
	APlayerController* PlayerController = nullptr;
	APC_C* APC = nullptr;
	APawn* Pawn = nullptr;
	ACharacter* Character = nullptr;
	AMec_C* MecChar = nullptr;
	UWorld* World = nullptr;
	ULevel* Level = nullptr;

	// Constructor to initialize variables safely
	Variables() {
		AutoSetVariables();
	}

	void AutoSetVariables() {

		// Get PlayerController first
		APlayerController* currentPC = Utils::GetPlayerController();
		this->APC = Utils::GetAPC();
		this->MecChar = this->APC ? this->APC->Mec_Ref : nullptr;
		if (!currentPC) {

			// Clear all dependent variables if PlayerController is null
			this->PlayerController = nullptr;
			this->Pawn = nullptr;
			this->Character = nullptr;
			this->MecChar = nullptr;
			this->World = Utils::GetWorldSafe();
			this->Level = this->World ? this->World->PersistentLevel : nullptr;
			return;
		}

		// Update PlayerController if changed
		if (this->PlayerController != currentPC) {
			this->PlayerController = currentPC;
			// Reset dependent variables when PlayerController changes
			this->Pawn = nullptr;
			this->Character = nullptr;
			this->MecChar = nullptr;
		}

		// Update Pawn
		if (this->PlayerController && this->Pawn != this->PlayerController->Pawn) {
			this->Pawn = this->PlayerController->Pawn;
			// Reset Character-dependent variables when Pawn changes
			this->Character = nullptr;
			this->MecChar = nullptr;
		}

		// Update Character
		if (this->PlayerController && this->Character != this->PlayerController->Character) {
			this->Character = this->PlayerController->Character;
			// Reset MecChar when Character changes
			this->MecChar = nullptr;
		}

		// Update MecChar
		if (this->Character) {
			AMec_C* newMecChar = (AMec_C*)this->Character;
			if (this->MecChar != newMecChar) {
				this->MecChar = newMecChar;
			}
		}

		// Update World
		UWorld* currentWorld = Utils::GetWorldSafe();
		if (this->World != currentWorld) {
			this->World = currentWorld;
			this->Level = nullptr; // Reset Level when World changes
		}

		// Update Level
		if (this->World && this->Level != this->World->PersistentLevel) {
			this->Level = this->World->PersistentLevel;
		}
	}
} inline GVars;
