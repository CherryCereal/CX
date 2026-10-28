// CXGameMode.cpp

#include "CXGameMode.h"
#include "CXChatGameState.h"
#include "CXPlayerController.h"
#include "CXPlayerState.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"

ACXGameMode::ACXGameMode()
{
	PlayerControllerClass = ACXPlayerController::StaticClass();
	PlayerStateClass = ACXPlayerState::StaticClass();
	GameStateClass = ACXChatGameState::StaticClass();
}
void ACXGameMode::BeginPlay(){ Super::BeginPlay(); if(ACXChatGameState* State=GetGameState<ACXChatGameState>()) State->SetMatchState(ECXMatchState::Waiting, WaitingDuration); }
void ACXGameMode::PostLogin(APlayerController* NewPlayer){ Super::PostLogin(NewPlayer); if(GetNumPlayers()==1) StartCountdown(ECXMatchState::Waiting, WaitingDuration); }
void ACXGameMode::StartCountdown(ECXMatchState State, int32 Duration){ if(ACXChatGameState* GS=GetGameState<ACXChatGameState>()){ GS->SetMatchState(State,Duration); GetWorldTimerManager().SetTimer(MatchTimer,this,&ThisClass::TickMatchTimer,1.f,true); } }
void ACXGameMode::TickMatchTimer(){ ACXChatGameState* GS=GetGameState<ACXChatGameState>(); if(!GS) return; GS->DecreaseRemainingTime(); if(GS->GetRemainingTime()>0) return; if(GS->GetMatchState()==ECXMatchState::Waiting) StartCountdown(ECXMatchState::Playing,PlayingDuration); else FinishMatch(); }
void ACXGameMode::FinishMatch(){ GetWorldTimerManager().ClearTimer(MatchTimer); ACXChatGameState* GS=GetGameState<ACXChatGameState>(); if(!GS) return; int32 Best=MIN_int32; TArray<FString> Winners; for(APlayerState* PS:GS->PlayerArray){ if(ACXPlayerState* CXPS=Cast<ACXPlayerState>(PS)){ if(CXPS->GetCoins()>Best){ Best=CXPS->GetCoins(); Winners={CXPS->GetPlayerName()}; }else if(CXPS->GetCoins()==Best) Winners.Add(CXPS->GetPlayerName()); } } GS->SetWinners(Winners); GS->SetMatchState(ECXMatchState::Finished,0); }

AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailablePlayerStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (IsPlayerStartAvailable(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	checkf(AvailablePlayerStarts.IsEmpty() == false, TEXT("There is no available Vehicle PlayerStart."));

	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());
	return Cast<AActor>(AvailablePlayerStarts[RandomIndex]);
}

bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (IsValid(PlayerStart) == false || IsValid(GetWorld()) == false)
	{
		return false;
	}

	const FTransform PlayerStartTransform = PlayerStart->GetActorTransform();
	const FVector CheckLocation = PlayerStartTransform.TransformPosition(VehicleSpawnCheckOffset);
	const FQuat CheckRotation = PlayerStartTransform.GetRotation();
	const FCollisionShape CheckShape = FCollisionShape::MakeBox(VehicleSpawnCheckHalfExtent);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);

	const bool bVehicleExists =
		GetWorld()->OverlapAnyTestByObjectType(
			CheckLocation,
			CheckRotation,
			ObjectQueryParams,
			CheckShape,
			QueryParams
		);

	return !bVehicleExists;
}
