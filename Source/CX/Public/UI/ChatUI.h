
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/EditableTextBox.h"

#include "ChatUI.generated.h"

struct FChatMessage;

/**
 * 
 */


UCLASS()
class CX_API UChatUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void OnNewChatMessage(const FString& Sender, const FString& Message);
	void SetChatHistory(const TArray<FChatMessage>& Messages);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UFUNCTION()
	void OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void FocusChatInput();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_ChatInput;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatScrollBox;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UMultiLineEditableTextBox> MultiLineEditableTextBox_Chat;
};
