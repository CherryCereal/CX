#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXMine.generated.h"

UCLASS()
class CX_API ACXMine : public AActor
{
	GENERATED_BODY()
public:
	ACXMine();
protected:
	UPROPERTY(VisibleAnywhere) TObjectPtr<class USphereComponent> Trigger;
	UPROPERTY(VisibleAnywhere) TObjectPtr<class UStaticMeshComponent> MineMesh;
	UPROPERTY(EditDefaultsOnly,Category=Effects) TObjectPtr<class UParticleSystem> ExplosionParticle;
	UPROPERTY(EditDefaultsOnly,Category=Effects) TObjectPtr<class USoundBase> ExplosionSound;
	bool bExploded = false;
	UFUNCTION() void OnVehicleOverlap(UPrimitiveComponent* Component,AActor* Other,UPrimitiveComponent* OtherComponent,int32 BodyIndex,bool bSweep,const FHitResult& Hit);
};
