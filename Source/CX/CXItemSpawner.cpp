#include "CXItemSpawner.h"
#include "CXItemBase.h"
#include "TimerManager.h"
ACXItemSpawner::ACXItemSpawner(){ bReplicates=true; }
void ACXItemSpawner::BeginPlay(){ Super::BeginPlay(); if(HasAuthority()) SpawnItem(); }
void ACXItemSpawner::SpawnItem(){ if(!HasAuthority()||!ItemClass) return; SpawnedItem=GetWorld()->SpawnActor<ACXItemBase>(ItemClass,GetActorTransform()); if(SpawnedItem) SpawnedItem->OnDestroyed.AddDynamic(this,&ThisClass::OnSpawnedItemDestroyed); }
void ACXItemSpawner::OnSpawnedItemDestroyed(AActor*){ SpawnedItem=nullptr; GetWorldTimerManager().SetTimer(RespawnTimer, this, &ThisClass::SpawnItem, RespawnDelay, false); }
