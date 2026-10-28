// CXPlayerController.cpp


#include "CXPlayerController.h"
#include "CXChatGameState.h"
#include "CXPlayerState.h"
#include "CXPawn.h"
#include "CXUI.h"
#include "UI/ChatUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

ACXPlayerController::ACXPlayerController()
{
	static ConstructorHelpers::FClassFinder<UChatUI> ChatUIBlueprintClass(TEXT("/Game/UI/UI_Chat"));
	if (ChatUIBlueprintClass.Succeeded())
	{
		ChatUIClass = ChatUIBlueprintClass.Class;
	}
}

void ACXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}
	
	// spawn the UI widget and add it to the viewport
	VehicleUI = CreateWidget<UCXUI>(this, VehicleUIClass);

	check(VehicleUI);

	VehicleUI->AddToViewport();

	if (ChatUIClass)
	{
		ChatUI = CreateWidget<UChatUI>(this, ChatUIClass);
		if (IsValid(ChatUI))
		{
			ChatUI->AddToViewport(1);
		}
	}
}

void ACXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);

		// optionally add the steering wheel context
		if (bUseSteeringWheelControls && SteeringWheelInputMappingContext)
		{
			Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
		}
	}
}

void ACXPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, VehiclePawn);
}

void ACXPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}

	if (IsLocalController() && IsValid(VehicleUI))
	{
		if (const ACXPlayerState* CXPlayerState = GetPlayerState<ACXPlayerState>())
		{
			VehicleUI->UpdateCoins(CXPlayerState->GetCoins());
			VehicleUI->UpdateBonusMaxSpeed(CXPlayerState->GetBonusMaxSpeed());
		}
		if (const ACXChatGameState* GameState = GetWorld()->GetGameState<ACXChatGameState>())
		{
			const bool bWon = PlayerState && GameState->GetWinners().Contains(PlayerState->GetPlayerName());
			VehicleUI->UpdateMatchState(GameState->GetMatchState(), GameState->GetRemainingTime(), bWon);
		}
	}
}

void ACXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<ACXPawn>(InPawn);
}

void ACXPlayerController::SubmitChatMessage(const FString& Message)
{
	if (Message.IsEmpty() == false)
	{
		ServerSubmitChatMessage(Message);
	}
}

void ACXPlayerController::ServerSubmitChatMessage_Implementation(const FString& Message)
{
	FString SanitizedMessage = Message;
	SanitizedMessage.TrimStartAndEndInline();
	SanitizedMessage = SanitizedMessage.Left(256);
	SanitizedMessage.ReplaceInline(TEXT("\r"), TEXT(" "));
	SanitizedMessage.ReplaceInline(TEXT("\n"), TEXT(" "));

	if (SanitizedMessage.IsEmpty())
	{
		return;
	}

	const FString Sender = PlayerState ? PlayerState->GetPlayerName() : TEXT("Player");
	if (ACXChatGameState* ChatGameState = GetWorld()->GetGameState<ACXChatGameState>())
	{
		ChatGameState->AddChatMessage(Sender, SanitizedMessage);
	}
}
