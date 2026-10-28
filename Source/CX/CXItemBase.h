#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXItemBase.generated.h"
class USphereComponent; class UParticleSystem; class USoundBase;
UCLASS(Abstract)
class CX_API ACXItemBase : public AActor
{
	GENERATED_BODY()
public: ACXItemBase();
protected:
	bool bCollected = false;
	UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent> Collision;
	UPROPERTY(EditDefaultsOnly, Category=Effects) TObjectPtr<UParticleSystem> CollectParticle;
	UPROPERTY(EditDefaultsOnly, Category=Effects) TObjectPtr<USoundBase> CollectSound;
	UFUNCTION() void OnCollected(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual bool GrantReward(class ACXPlayerState* PlayerState) PURE_VIRTUAL(ACXItemBase::GrantReward, return false;);
};
