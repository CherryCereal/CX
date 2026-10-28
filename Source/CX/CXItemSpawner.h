#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXItemSpawner.generated.h"
UCLASS() class CX_API ACXItemSpawner : public AActor { GENERATED_BODY() public: ACXItemSpawner(); protected: virtual void BeginPlay() override; UPROPERTY(EditInstanceOnly,Category=Spawn) TSubclassOf<class ACXItemBase> ItemClass; UPROPERTY(VisibleInstanceOnly,Category=Spawn) TObjectPtr<ACXItemBase> SpawnedItem; UPROPERTY(EditInstanceOnly,Category=Spawn) float RespawnDelay=2.f; FTimerHandle RespawnTimer; void SpawnItem(); UFUNCTION() void OnSpawnedItemDestroyed(AActor* DestroyedActor); };
