#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CXPlayerState.generated.h"

UCLASS()
class CX_API ACXPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACXPlayerState();
	virtual void Tick(float DeltaSeconds) override;
	void ApplyWheelPenalty(float Amount);
	int32 GetCoins() const { return Coins; }
	float GetBonusMaxSpeed() const { return BonusMaxSpeed; }
	bool AddCoins(int32 Amount);
	bool SpendCoins(int32 Amount);
	bool AddBonusMaxSpeed(float Amount);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION() void OnRep_Coins();
	UFUNCTION() void OnRep_BonusMaxSpeed();

private:
	float WheelPenalty = 0.0f;
	UPROPERTY(ReplicatedUsing = OnRep_Coins, VisibleInstanceOnly, Category = Rewards)
	int32 Coins = 0;
	UPROPERTY(ReplicatedUsing = OnRep_BonusMaxSpeed, VisibleInstanceOnly, Category = Rewards)
	float BonusMaxSpeed = 0.0f;
};
