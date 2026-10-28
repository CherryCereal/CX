#include "CXItemBase.h"
#include "CXPlayerState.h"
#include "CXPawn.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CXChatGameState.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
ACXItemBase::ACXItemBase(){ bReplicates=true; SetReplicateMovement(true); SetNetUpdateFrequency(1.0f); Collision=CreateDefaultSubobject<USphereComponent>(TEXT("Collision")); SetRootComponent(Collision); Collision->SetSphereRadius(100.f); Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic")); Collision->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::OnCollected); }
void ACXItemBase::OnCollected(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!HasAuthority() || bCollected) return;
	ACXPawn* Pawn = Cast<ACXPawn>(OtherActor);
	ACXPlayerState* State = Pawn && Pawn->GetController() ? Pawn->GetController()->GetPlayerState<ACXPlayerState>() : nullptr;
	if (!State) return;
	bCollected = true;
	if (!GrantReward(State)) { bCollected = false; return; }
	if (ACXChatGameState* GS = GetWorld()->GetGameState<ACXChatGameState>())
		GS->MulticastItemEffects(CollectParticle, CollectSound, GetActorLocation(), GetActorRotation());
	if (!CollectParticle || !CollectSound)
		UE_LOG(LogTemp, Warning, TEXT("%s: CollectParticle=%s, CollectSound=%s. Check spawned ItemClass defaults."), *GetName(), *GetNameSafe(CollectParticle), *GetNameSafe(CollectSound));
	Destroy();
}
