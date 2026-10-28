#include "CXMine.h"
#include "CXPawn.h"
#include "CXChatGameState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

ACXMine::ACXMine()
{
	bReplicates=true;
	Trigger=CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetSphereRadius(100.f);
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Trigger->OnComponentBeginOverlap.AddDynamic(this,&ThisClass::OnVehicleOverlap);
	MineMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
	MineMesh->SetupAttachment(Trigger);
	MineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MineMesh->SetRelativeScale3D(FVector(.5f,.5f,.1f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> Particle(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	static ConstructorHelpers::FObjectFinder<USoundBase> Sound(TEXT("/Game/StarterContent/Audio/Explosion02.Explosion02"));
	MineMesh->SetStaticMesh(Mesh.Object);
	ExplosionParticle=Particle.Object;
	ExplosionSound=Sound.Object;
}

void ACXMine::OnVehicleOverlap(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,int32,bool,const FHitResult&)
{
	if (!HasAuthority() || bExploded || Other==GetOwner()) return;
	ACXPawn* Victim=Cast<ACXPawn>(Other);
	if (!Victim) return;
	bExploded=true;
	Victim->DisableThrottleForMine();
	if (ACXChatGameState* GS=GetWorld()->GetGameState<ACXChatGameState>()) GS->MulticastItemEffects(ExplosionParticle,ExplosionSound,GetActorLocation(),GetActorRotation());
	Destroy();
}
