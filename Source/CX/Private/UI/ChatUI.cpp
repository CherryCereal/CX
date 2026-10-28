#include "UI/ChatUI.h"

#include "../../CXChatGameState.h"
#include "../../CXPlayerController.h"
#include "Components/ScrollBox.h"



void UChatUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(MultiLineEditableTextBox_Chat) == false)
	{
		MultiLineEditableTextBox_Chat = Cast<UMultiLineEditableTextBox>(GetWidgetFromName(TEXT("MultiLineEditableTextBox_Chat")));
	}

	if (IsValid(EditableTextBox_ChatInput) && EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &UChatUI::OnChatInputTextCommitted);
	}

	if (IsValid(MultiLineEditableTextBox_Chat))
	{
		MultiLineEditableTextBox_Chat->SetIsReadOnly(true);
	}

	if (ACXChatGameState* ChatGameState = GetWorld()->GetGameState<ACXChatGameState>())
	{
		ChatGameState->OnChatMessageAdded.AddUObject(this, &ThisClass::OnNewChatMessage);
		ChatGameState->OnChatHistoryChanged.AddUObject(this, &ThisClass::SetChatHistory);
		SetChatHistory(ChatGameState->GetChatMessages());
	}
}

void UChatUI::NativeDestruct()
{

	if (IsValid(EditableTextBox_ChatInput) && EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &UChatUI::OnChatInputTextCommitted);
	}

	if (ACXChatGameState* ChatGameState = GetWorld()->GetGameState<ACXChatGameState>())
	{
		ChatGameState->OnChatMessageAdded.RemoveAll(this);
		ChatGameState->OnChatHistoryChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UChatUI::OnNewChatMessage(const FString& Sender, const FString& Message)
{
	if (IsValid(MultiLineEditableTextBox_Chat) == false)
	{
		return;
	}

	const FString ExistingText = MultiLineEditableTextBox_Chat->GetText().ToString();
	const FString NewLine = FString::Printf(TEXT("%s: %s"), *Sender, *Message);
	const FString CombinedText = ExistingText.IsEmpty() ? NewLine : ExistingText + LINE_TERMINATOR + NewLine;
	MultiLineEditableTextBox_Chat->SetText(FText::FromString(CombinedText));

	if (IsValid(ChatScrollBox))
	{
		ChatScrollBox->ScrollToEnd();
	}
}

void UChatUI::SetChatHistory(const TArray<FChatMessage>& Messages)
{
	if (IsValid(MultiLineEditableTextBox_Chat) == false)
	{
		return;
	}

	TArray<FString> Lines;
	Lines.Reserve(Messages.Num());
	for (const FChatMessage& ChatMessage : Messages)
	{
		Lines.Add(FString::Printf(TEXT("%s: %s"), *ChatMessage.Sender, *ChatMessage.Message));
	}

	MultiLineEditableTextBox_Chat->SetText(FText::FromString(FString::Join(Lines, LINE_TERMINATOR)));

	if (IsValid(ChatScrollBox))
	{
		ChatScrollBox->ScrollToEnd();
	}
}

void UChatUI::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if(CommitMethod == ETextCommit::OnEnter)
	{
		FString Message = Text.ToString();
		Message.TrimStartAndEndInline();

		if(Message.IsEmpty() == false)
		{
			EditableTextBox_ChatInput->SetText(FText::GetEmpty());

			if(ACXPlayerController* PC = GetOwningPlayer<ACXPlayerController>())
			{
				PC->SubmitChatMessage(Message);
			}
		}
	}
}

void UChatUI::FocusChatInput()
{
	if (IsValid(EditableTextBox_ChatInput))
	{
		EditableTextBox_ChatInput->SetKeyboardFocus();
	}
}
