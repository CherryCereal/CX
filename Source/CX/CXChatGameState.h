#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXChatGameState.generated.h"

USTRUCT()
struct FChatMessage
{
	GENERATED_BODY()

	UPROPERTY()
	FString Sender;

	UPROPERTY()
	FString Message;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChatMessageAdded, const FString& /* Sender */, const FString& /* Message */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChatHistoryChanged, const TArray<FChatMessage>& /* Messages */);

UENUM(BlueprintType)
enum class ECXMatchState : uint8 { Waiting, Playing, Finished };

UCLASS()
class CX_API ACXChatGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ACXChatGameState();

	void AddChatMessage(const FString& Sender, const FString& Message);
	const TArray<FChatMessage>& GetChatMessages() const { return ChatMessages; }
	ECXMatchState GetMatchState() const { return MatchState; }
	int32 GetRemainingTime() const { return RemainingTime; }
	const TArray<FString>& GetWinners() const { return Winners; }
	void SetMatchState(ECXMatchState NewState, int32 NewRemainingTime);
	void DecreaseRemainingTime();
	void SetWinners(const TArray<FString>& NewWinners);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastItemEffects(class UParticleSystem* Particle, class USoundBase* Sound, FVector Location, FRotator Rotation);

	FOnChatMessageAdded OnChatMessageAdded;
	FOnChatHistoryChanged OnChatHistoryChanged;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ChatMessages();
	UFUNCTION() void OnRep_MatchState();
	UFUNCTION() void OnRep_RemainingTime();
	UFUNCTION() void OnRep_Winners();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceiveChatMessage(const FString& Sender, const FString& Message);

private:
	UPROPERTY(ReplicatedUsing = OnRep_ChatMessages)
	TArray<FChatMessage> ChatMessages;
	UPROPERTY(ReplicatedUsing=OnRep_MatchState) ECXMatchState MatchState = ECXMatchState::Waiting;
	UPROPERTY(ReplicatedUsing=OnRep_RemainingTime) int32 RemainingTime = 0;
	UPROPERTY(ReplicatedUsing=OnRep_Winners) TArray<FString> Winners;

	UPROPERTY(EditDefaultsOnly, Category = Chat, meta = (ClampMin = "1"))
	int32 MaximumStoredMessages = 50;
};
