#include <cstdio>

#include <imgui.h>
#include <numbers>
#include "SDK/Engine_classes.hpp"
#include "Utils.h"

UWorld* Utils::GetWorldSafe()
{
    UWorld* World = nullptr;
    int i = 0;
    while (i < 50) {
        i++;
        UEngine* Engine = UEngine::GetEngine();
        if (!Engine) {
            printf("[Error] Engine not found!\n");
            continue;
        }

        UGameViewportClient* Viewport = Engine->GameViewport;
        if (!Viewport) {
            printf("[Error] GameViewport not found!\n");
            continue;
        }

        World = Viewport->World;
        if (!World) {
            printf("[Error] World not found!\n");
            continue;
        }
        break; // Successfully obtained World
    }
    return World;
}

APlayerController* Utils::GetPlayerController()
{
    int i = 0;
    APlayerController* PlayerController = nullptr;

    while (i < 50) {
        i++;
        UWorld* World = GetWorldSafe();
        if (!World) return nullptr; // Error already logged in GetWorldSafe
        UGameInstance* GameInstance = World->OwningGameInstance;
        if (!GameInstance) {
            printf("[Error] GameInstance not found!\n");
        }
        if (GameInstance->LocalPlayers.Num() <= 0) {
            printf("[Error] No LocalPlayers in GameInstance!\n");
        }
        ULocalPlayer* LocalPlayer = GameInstance->LocalPlayers[0];
        if (!LocalPlayer) {
            printf("[Error] LocalPlayer is null!\n");
        }
        PlayerController = LocalPlayer->PlayerController;
        if (!PlayerController) {
            printf("[Error] PlayerController not found!\n");
        }
    }
    return PlayerController;
}

APC_C* Utils::GetAPC()
{
	UWorld* World = GetWorldSafe();
    if (!World) return nullptr; // Error already logged in GetWorldSafe
    UGameInstance* GameInstance = World->OwningGameInstance;
    if (!GameInstance) {
        printf("[Error] GameInstance not found!\n");
        return nullptr;
    }
    if (GameInstance->LocalPlayers.Num() <= 0) {
        printf("[Error] No LocalPlayers in GameInstance!\n");
        return nullptr;
    }
    ULocalPlayer* LocalPlayer = GameInstance->LocalPlayers[0];
    if (!LocalPlayer) {
        printf("[Error] LocalPlayer is null!\n");
        return nullptr;
    }
    APlayerController* PlayerController = LocalPlayer->PlayerController;
    if (!PlayerController) {
        printf("[Error] PlayerController not found!\n");
        return nullptr;
    }
    APC_C* APC = (APC_C*)PlayerController;
    if (!APC) {
        printf("[Error] APC_C cast failed!\n");
        return nullptr;
    }
	return APC;
}

void Utils::PrintActors(const char* Exclude)
{
	UWorld* World = GetWorldSafe();
    ULevel* Level = World->PersistentLevel;
    if (Level)
    {
        TArray<AActor*> Actors = Level->Actors;
        for (int i = 0; i < Actors.Num(); i++)
        {
            AActor* Actor = Actors[i];
            if (Actor)
            {
                if (Exclude && Actor->GetName().find(Exclude) != std::string::npos)
                    continue;

                printf("Actor %d: %s - Class: %s\n", i, Actor->GetName().c_str(), Actor->Class->Name.ToString().c_str());
            }
        }
    }
}