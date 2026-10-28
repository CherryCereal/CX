#include "CXPlayerState.h"
#include "Net/UnrealNetwork.h"

ACXPlayerState::ACXPlayerState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACXPlayerState::ApplyWheelPenalty(float Amount)
{
	if (!HasAuthority() || !FMath::IsFinite(Amount) || Amount <= 0.f) return;
	// Keep earned buffs separate: only the temporary negative contribution recovers.
	BonusMaxSpeed -= Amount;
	WheelPenalty -= Amount;
	ForceNetUpdate();
}

void ACXPlayerState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!HasAuthority() || WheelPenalty >= 0.f) return;
	const float Recovery = FMath::Min(-WheelPenalty, FMath::Max(0.f, DeltaSeconds) * 5.f);
	WheelPenalty = FMath::Min(0.f, WheelPenalty + Recovery);
	BonusMaxSpeed += Recovery;
}

bool ACXPlayerState::AddCoins(const int32 Amount)
{
	if (!HasAuthority() || Amount <= 0) return false;
	Coins = FMath::Min(MAX_int32, Coins + Amount);
	ForceNetUpdate();
	return true;
}

bool ACXPlayerState::SpendCoins(const int32 Amount)
{
	if (!HasAuthority() || Amount <= 0 || Coins < Amount) return false;
	Coins -= Amount;
	ForceNetUpdate();
	return true;
}

bool ACXPlayerState::AddBonusMaxSpeed(const float Amount)
{
	if (!HasAuthority() || Amount <= 0.0f) return false;
	BonusMaxSpeed += Amount;
	ForceNetUpdate();
	return true;
}

void ACXPlayerState::OnRep_Coins() {}
void ACXPlayerState::OnRep_BonusMaxSpeed() {}

void ACXPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ACXPlayerState, Coins, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ACXPlayerState, BonusMaxSpeed, COND_OwnerOnly);
}
