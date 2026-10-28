#include "CXChatGameState.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

void ACXChatGameState::MulticastItemEffects_Implementation(UParticleSystem* Particle, USoundBase* Sound, FVector Location, FRotator Rotation)
{
	if (GetNetMode() == NM_DedicatedServer) return;
	if (Particle) UGameplayStatics::SpawnEmitterAtLocation(this, Particle, Location, Rotation);
	if (Sound) UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
}

ACXChatGameState::ACXChatGameState()
{
	bReplicates = true;
}

void ACXChatGameState::AddChatMessage(const FString& Sender, const FString& Message)
{
	check(HasAuthority());

	FChatMessage& NewMessage = ChatMessages.AddDefaulted_GetRef();
	NewMessage.Sender = Sender;
	NewMessage.Message = Message;

	if (ChatMessages.Num() > MaximumStoredMessages)
	{
		ChatMessages.RemoveAt(0, ChatMessages.Num() - MaximumStoredMessages);
	}

	MulticastReceiveChatMessage(Sender, Message);
}

void ACXChatGameState::OnRep_ChatMessages()
{
	OnChatHistoryChanged.Broadcast(ChatMessages);
}
void ACXChatGameState::SetMatchState(ECXMatchState NewState, int32 NewRemainingTime){ check(HasAuthority()); MatchState=NewState; RemainingTime=FMath::Max(0,NewRemainingTime); ForceNetUpdate(); }
void ACXChatGameState::DecreaseRemainingTime(){ check(HasAuthority()); RemainingTime=FMath::Max(0,RemainingTime-1); ForceNetUpdate(); }
void ACXChatGameState::SetWinners(const TArray<FString>& NewWinners){ check(HasAuthority()); Winners=NewWinners; ForceNetUpdate(); }
void ACXChatGameState::OnRep_MatchState(){}
void ACXChatGameState::OnRep_RemainingTime(){}
void ACXChatGameState::OnRep_Winners(){}

void ACXChatGameState::MulticastReceiveChatMessage_Implementation(const FString& Sender, const FString& Message)
{
	OnChatMessageAdded.Broadcast(Sender, Message);
}

void ACXChatGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, ChatMessages);
	DOREPLIFETIME(ThisClass, MatchState);
	DOREPLIFETIME(ThisClass, RemainingTime);
	DOREPLIFETIME(ThisClass, Winners);
}
