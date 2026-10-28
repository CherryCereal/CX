// Copyright Epic Games, Inc. All Rights Reserved.


#include "CXUI.h"
#include "Components/TextBlock.h"

namespace
{
 void SetHUDText(UCXUI* UI, const FName Name, const FText& Text)
 {
  if (UTextBlock* Label = Cast<UTextBlock>(UI->GetWidgetFromName(Name))) Label->SetText(Text);
 }
}
void UCXUI::DisplayCoins(int32 NewCoins)
{
 SetHUDText(this, TEXT("CoinsText"), FText::Format(NSLOCTEXT("CXHUD","Coins","Coins: {0}"), FText::AsNumber(NewCoins)));
}
void UCXUI::DisplayBonusMaxSpeed(float NewBonusMaxSpeed)
{
 SetHUDText(this, TEXT("BonusMaxSpeedText"), FText::Format(NSLOCTEXT("CXHUD","Bonus","Bonus Max Speed: +{0}"), FText::AsNumber(NewBonusMaxSpeed)));
}
void UCXUI::DisplayMatchState(ECXMatchState NewMatchState, int32 NewRemainingTime)
{
 const TCHAR* Label = NewMatchState == ECXMatchState::Waiting ? TEXT("Waiting") : NewMatchState == ECXMatchState::Playing ? TEXT("Playing") : TEXT("Finished");
 SetHUDText(this,TEXT("MatchStateText"),FText::FromString(Label));
 SetHUDText(this,TEXT("RemainingTimeText"),FText::FromString(FString::Printf(TEXT("Time: %02d:%02d"),FMath::Max(0,NewRemainingTime)/60,FMath::Max(0,NewRemainingTime)%60)));
 if(NewMatchState != ECXMatchState::Finished) SetHUDText(this,TEXT("MatchResultText"),FText::GetEmpty());
}
void UCXUI::DisplayMatchResult(bool bWon)
{
 SetHUDText(this,TEXT("MatchResultText"),bWon ? NSLOCTEXT("CXHUD","Win","VICTORY") : NSLOCTEXT("CXHUD","Lose","DEFEAT"));
}

void UCXUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
}

void UCXUI::UpdateGear(int32 NewGear)
{
	// call the Blueprint handler
	OnGearUpdate(NewGear);
}

void UCXUI::UpdateCoins(int32 NewCoins)
{
	OnCoinsUpdate(NewCoins);
}

void UCXUI::UpdateBonusMaxSpeed(float NewBonusMaxSpeed)
{
	OnBonusMaxSpeedUpdate(NewBonusMaxSpeed);
}
void UCXUI::UpdateMatchState(ECXMatchState NewMatchState, int32 NewRemainingTime, bool bLocalPlayerWon){ OnMatchStateUpdate(NewMatchState,NewRemainingTime); if(NewMatchState==ECXMatchState::Finished) OnMatchResult(bLocalPlayerWon); }
