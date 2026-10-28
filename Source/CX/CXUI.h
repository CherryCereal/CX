// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CXChatGameState.h"
#include "CXUI.generated.h"

/**
 *  Simple Vehicle HUD class
 *  Displays the current speed and gear.
 *  Widget setup is handled in a Blueprint subclass.
 */
UCLASS(abstract)
class CX_API UCXUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	/** Controls the display of speed in Km/h or MPH */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Vehicle)
	bool bIsMPH = false;

public:
	UFUNCTION(BlueprintCallable, Category=HUD) void DisplayCoins(int32 NewCoins);
	UFUNCTION(BlueprintCallable, Category=HUD) void DisplayBonusMaxSpeed(float NewBonusMaxSpeed);
	UFUNCTION(BlueprintCallable, Category=HUD) void DisplayMatchState(ECXMatchState NewMatchState, int32 NewRemainingTime);
	UFUNCTION(BlueprintCallable, Category=HUD) void DisplayMatchResult(bool bWon);

	/** Called to update the speed display */
	void UpdateSpeed(float NewSpeed);

	/** Called to update the gear display */
	void UpdateGear(int32 NewGear);
	void UpdateCoins(int32 NewCoins);
	void UpdateBonusMaxSpeed(float NewBonusMaxSpeed);
	void UpdateMatchState(ECXMatchState NewMatchState, int32 NewRemainingTime, bool bLocalPlayerWon);

protected:

	/** Implemented in Blueprint to display the new speed */
	UFUNCTION(BlueprintImplementableEvent, Category = Vehicle)
	void OnSpeedUpdate(float NewSpeed);

	/** Implemented in Blueprint to display the new gear */
	UFUNCTION(BlueprintImplementableEvent, Category = Vehicle)
	void OnGearUpdate(int32 NewGear);

	UFUNCTION(BlueprintImplementableEvent, Category = Rewards)
	void OnCoinsUpdate(int32 NewCoins);

	UFUNCTION(BlueprintImplementableEvent, Category = Rewards)
	void OnBonusMaxSpeedUpdate(float NewBonusMaxSpeed);
	UFUNCTION(BlueprintImplementableEvent, Category=Match) void OnMatchStateUpdate(ECXMatchState NewMatchState, int32 NewRemainingTime);
	UFUNCTION(BlueprintImplementableEvent, Category=Match) void OnMatchResult(bool bWon);
};
