#include "ConfigureHUDCommandlet.h"
#include "../CX/CXUI.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "EdGraphSchema_K2.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "../CX/CXPlayerState.h"
#include "../CX/CXMine.h"
#include "Engine/World.h"

int32 UConfigureHUDCommandlet::Main(const FString& Params)
{
	if (Params.Contains(TEXT("VerifyMine")))
	{
		const UWorld::InitializationValues Init=UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false);
		UWorld* World=UWorld::CreateWorld(EWorldType::Game,false,NAME_None,nullptr,true,ERHIFeatureLevel::Num,&Init);
		ACXPlayerState* State=World->SpawnActor<ACXPlayerState>();
		State->ApplyWheelPenalty(10.f);
		bool bPassed=FMath::IsNearlyEqual(State->GetBonusMaxSpeed(),-10.f);
		State->Tick(1.f);
		bPassed &= FMath::IsNearlyEqual(State->GetBonusMaxSpeed(),-5.f);
		State->Tick(100.f);
		bPassed &= FMath::IsNearlyZero(State->GetBonusMaxSpeed());
		State->AddBonusMaxSpeed(20.f);
		State->ApplyWheelPenalty(10.f);
		State->Tick(100.f);
		bPassed &= FMath::IsNearlyEqual(State->GetBonusMaxSpeed(),20.f);
		AActor* Owner=World->SpawnActor<AActor>();
		FActorSpawnParameters SpawnParams; SpawnParams.Owner=Owner;
		ACXMine* Mine=World->SpawnActor<ACXMine>(ACXMine::StaticClass(),FTransform::Identity,SpawnParams);
		bPassed &= Mine && Mine->GetOwner()==Owner && Mine->GetIsReplicated();
		UInputAction* Action=LoadObject<UInputAction>(nullptr,TEXT("/Game/VehicleTemplate/Input/Actions/IA_LayMine.IA_LayMine"));
		UInputMappingContext* Mapping=LoadObject<UInputMappingContext>(nullptr,TEXT("/Game/VehicleTemplate/Input/IMC_Default.IMC_Default"));
		bool bMapped=false;
		if (Mapping) for (const FEnhancedActionKeyMapping& Entry:Mapping->GetMappings()) bMapped |= Entry.Action==Action && Entry.Key==EKeys::F;
		bPassed &= Action && bMapped;
		for (const TCHAR* Path:{TEXT("/Game/VehicleTemplate/Blueprints/SportsCar/BP_SportsCar_Pawn.BP_SportsCar_Pawn_C"),TEXT("/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn.BP_OffroadCar_Pawn_C")})
		{
			UClass* Class=LoadObject<UClass>(nullptr,Path);
			FObjectPropertyBase* Property=Class?FindFProperty<FObjectPropertyBase>(Class,TEXT("MineAction")):nullptr;
			bPassed &= Property && Property->GetObjectPropertyValue_InContainer(Class->GetDefaultObject())==Action;
		}
		World->DestroyWorld(false);
		UE_LOG(LogTemp,Display,TEXT("Mine checks: %s (negative recovery clamp, earned buff preservation, owner, replication, persisted F mapping and both pawns)."),bPassed?TEXT("PASS"):TEXT("FAIL"));
		return bPassed?0:20;
	}
	if (Params.Contains(TEXT("ConfigureMine")))
	{
		auto SaveAsset=[](UObject* Asset)
		{
			const FString Filename=FPackageName::LongPackageNameToFilename(Asset->GetOutermost()->GetName(),FPackageName::GetAssetPackageExtension());
			FSavePackageArgs Args; Args.TopLevelFlags=RF_Public|RF_Standalone;
			return UPackage::SavePackage(Asset->GetOutermost(),Asset,*Filename,Args);
		};
		UInputAction* Action=LoadObject<UInputAction>(nullptr,TEXT("/Game/VehicleTemplate/Input/Actions/IA_LayMine.IA_LayMine"));
		if (!Action)
		{
			UPackage* Package=CreatePackage(TEXT("/Game/VehicleTemplate/Input/Actions/IA_LayMine"));
			Action=NewObject<UInputAction>(Package,TEXT("IA_LayMine"),RF_Public|RF_Standalone);
			Action->ValueType=EInputActionValueType::Boolean;
			if (!SaveAsset(Action)) return 10;
		}
		UInputMappingContext* Mapping=LoadObject<UInputMappingContext>(nullptr,TEXT("/Game/VehicleTemplate/Input/IMC_Default.IMC_Default"));
		if (!Mapping) return 11;
		bool bMapped=false;
		for (const FEnhancedActionKeyMapping& Entry:Mapping->GetMappings())
		{
			if (Entry.Key==EKeys::F && Entry.Action!=Action) { UE_LOG(LogTemp,Error,TEXT("F key already assigned")); return 12; }
			if (Entry.Key==EKeys::F && Entry.Action==Action) bMapped=true;
		}
		if (!bMapped) Mapping->MapKey(Action,EKeys::F);
		if (!SaveAsset(Mapping)) return 13;
		const TCHAR* Vehicles[]={TEXT("/Game/VehicleTemplate/Blueprints/SportsCar/BP_SportsCar_Pawn.BP_SportsCar_Pawn"),TEXT("/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn.BP_OffroadCar_Pawn")};
		for (const TCHAR* Path:Vehicles)
		{
			UBlueprint* Vehicle=LoadObject<UBlueprint>(nullptr,Path);
			if (!Vehicle || !Vehicle->GeneratedClass) return 14;
			FObjectPropertyBase* Property=FindFProperty<FObjectPropertyBase>(Vehicle->GeneratedClass,TEXT("MineAction"));
			if (!Property) return 15;
			Property->SetObjectPropertyValue_InContainer(Vehicle->GeneratedClass->GetDefaultObject(),Action);
			FBlueprintEditorUtils::MarkBlueprintAsModified(Vehicle);
			FKismetEditorUtilities::CompileBlueprint(Vehicle);
			if (Vehicle->Status==BS_Error || !SaveAsset(Vehicle)) return 16;
		}
		UE_LOG(LogTemp,Display,TEXT("Mine input configured: IA_LayMine, F key, sports and offroad pawns."));
		return 0;
	}
	if (Params.Contains(TEXT("AuditEffects")))
	{
		const TCHAR* Paths[] = { TEXT("/Game/VehicleTemplate/Blueprints/BP_CXCoinItem.BP_CXCoinItem_C"), TEXT("/Game/VehicleTemplate/Blueprints/BP_CXSpeedBuffItem.BP_CXSpeedBuffItem_C"), TEXT("/Game/VehicleTemplate/Blueprints/SportsCar/BP_SportsCar_Pawn.BP_SportsCar_Pawn_C"), TEXT("/Game/VehicleTemplate/Blueprints/OffroadCar/BP_OffroadCar_Pawn.BP_OffroadCar_Pawn_C") };
		for (const TCHAR* Path : Paths)
		{
			UClass* Class = LoadObject<UClass>(nullptr, Path);
			if (!Class) continue;
			for (TFieldIterator<FObjectPropertyBase> It(Class); It; ++It)
			{
				const FString Name=It->GetName();
				if (Name==TEXT("CollectParticle") || Name==TEXT("CollectSound") || Name==TEXT("AttackAction"))
					UE_LOG(LogTemp,Display,TEXT("AUDIT %s %s = %s"),Path,*Name,*GetPathNameSafe(It->GetObjectPropertyValue_InContainer(Class->GetDefaultObject())));
			}
		}
		return 0;
	}
 UWidgetBlueprint* BP=LoadObject<UWidgetBlueprint>(nullptr,TEXT("/Game/VehicleTemplate/Blueprints/W_VehicleAdvUI.W_VehicleAdvUI"));
 if(!BP || !BP->WidgetTree) return 1;
 UOverlay* Root=Cast<UOverlay>(BP->WidgetTree->RootWidget);
 if(!Root) { UE_LOG(LogTemp,Error,TEXT("Expected Overlay HUD root")); return 2; }
 const FString Filename=FPackageName::LongPackageNameToFilename(BP->GetOutermost()->GetName(),FPackageName::GetAssetPackageExtension());
 const FString Backup=FPaths::ProjectSavedDir()/TEXT("HUDBackup/W_VehicleAdvUI.uasset");
 IFileManager::Get().MakeDirectory(*FPaths::GetPath(Backup),true);
 if(!IFileManager::Get().FileExists(*Backup)) IFileManager::Get().Copy(*Backup,*Filename);
 UVerticalBox* Panel=Cast<UVerticalBox>(BP->WidgetTree->FindWidget(TEXT("MatchHUDPanel")));
 if(!Panel)
 {
  Panel=BP->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),TEXT("MatchHUDPanel"));
  UOverlaySlot* Slot=Root->AddChildToOverlay(Panel);
  Slot->SetHorizontalAlignment(HAlign_Right); Slot->SetVerticalAlignment(VAlign_Top); Slot->SetPadding(FMargin(24.f));
 }
 const TCHAR* Names[]={TEXT("CoinsText"),TEXT("BonusMaxSpeedText"),TEXT("MatchStateText"),TEXT("RemainingTimeText"),TEXT("MatchResultText")};
 const TCHAR* Defaults[]={TEXT("Coins: 0"),TEXT("Bonus Max Speed: +0"),TEXT("Waiting"),TEXT("Time: 00:10"),TEXT("")};
 for(int32 i=0;i<5;++i)
 {
  UTextBlock* Text=Cast<UTextBlock>(BP->WidgetTree->FindWidget(Names[i]));
  if(!Text)
  {
   Text=BP->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Names[i]);
   Panel->AddChildToVerticalBox(Text)->SetPadding(FMargin(6.f));
  }
  Text->SetText(FText::FromString(Defaults[i]));
  FSlateFontInfo Font=Text->GetFont(); Font.Size=i==4?32:24; Text->SetFont(Font);
  Text->SetShadowOffset(FVector2D(2,2)); Text->SetShadowColorAndOpacity(FLinearColor::Black);
  Text->SetColorAndOpacity(FSlateColor(i==4?FLinearColor(1,.8f,.1f):FLinearColor::White));
 }
 UEdGraph* Graph=BP->UbergraphPages.Num()?BP->UbergraphPages[0]:nullptr;
 if(!Graph) return 3;
 const TCHAR* Events[]={TEXT("OnCoinsUpdate"),TEXT("OnBonusMaxSpeedUpdate"),TEXT("OnMatchStateUpdate"),TEXT("OnMatchResult")};
 const TCHAR* Functions[]={TEXT("DisplayCoins"),TEXT("DisplayBonusMaxSpeed"),TEXT("DisplayMatchState"),TEXT("DisplayMatchResult")};
 const UEdGraphSchema_K2* Schema=GetDefault<UEdGraphSchema_K2>();
 for(int32 i=0;i<4;++i)
 {
  UK2Node_Event* Event=nullptr;
  for(UEdGraphNode* Node:Graph->Nodes) if(UK2Node_Event* E=Cast<UK2Node_Event>(Node)) if(E->EventReference.GetMemberName()==FName(Events[i])) Event=E;
  if(Event && Event->FindPinChecked(UEdGraphSchema_K2::PN_Then)->LinkedTo.Num()) { UE_LOG(LogTemp,Error,TEXT("Existing event wiring: %s"),Events[i]); return 4; }
  if(!Event)
  {
   Event=NewObject<UK2Node_Event>(Graph); Graph->AddNode(Event,false,false); Event->CreateNewGuid();
   Event->EventReference.SetExternalMember(Events[i],UCXUI::StaticClass()); Event->bOverrideFunction=true;
   Event->NodePosX=0; Event->NodePosY=800+i*250; Event->AllocateDefaultPins();
  }
  UK2Node_CallFunction* Call=NewObject<UK2Node_CallFunction>(Graph); Graph->AddNode(Call,false,false); Call->CreateNewGuid();
  Call->SetFromFunction(UCXUI::StaticClass()->FindFunctionByName(Functions[i])); Call->NodePosX=400; Call->NodePosY=Event->NodePosY; Call->AllocateDefaultPins();
  if(!Schema->TryCreateConnection(Event->FindPinChecked(UEdGraphSchema_K2::PN_Then),Call->GetExecPin())) return 5;
  for(UEdGraphPin* Pin:Event->Pins) if(Pin->Direction==EGPD_Output && Pin->PinType.PinCategory!=UEdGraphSchema_K2::PC_Exec)
   if(UEdGraphPin* Input=Call->FindPin(Pin->PinName,EGPD_Input)) if(!Schema->TryCreateConnection(Pin,Input)) return 6;
 }
 FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
 FKismetEditorUtilities::CompileBlueprint(BP);
 if(BP->Status==BS_Error) return 7;
 FSavePackageArgs Args; Args.TopLevelFlags=RF_Public|RF_Standalone; Args.SaveFlags=SAVE_NoError;
 if(!UPackage::SavePackage(BP->GetOutermost(),BP,*Filename,Args)) return 8;
 UE_LOG(LogTemp,Display,TEXT("HUD configured: five TextBlocks and four event graphs. Compile and save successful."));
 return 0;
}
