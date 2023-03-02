#include "UEOpenWorldDevPanel/Public/OpenWorldHelper.h"

#include "DesktopPlatformModule.h"
#include "EngineUtils.h"
#include "LevelEditorViewport.h"
#include "Engine/PostProcessVolume.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "FOpenWorldHelper"

FOpenWorldHelper* FOpenWorldHelper::_Instance = nullptr;

FString FOpenWorldHelper::GetSelectJsonPath()
{
	if (SelectJsonPath.IsEmpty())
	{
		if (GCurrentLevelEditingViewportClient)
		{
			UWorld* CurrentWorld = GCurrentLevelEditingViewportClient->GetWorld();
			if (CurrentWorld)
			{
				SelectJsonPath = FPaths::ProjectPluginsDir() + "UEOpenWorldDevPanel/Content/Maps/" + CurrentWorld->GetMapName() + ".json";
			}
		}
	}

	return SelectJsonPath;
}

void FOpenWorldHelper::SetSelectJsonPath(FString InJsonPath)
{
	SelectJsonPath = InJsonPath;
}

bool FOpenWorldHelper::WriteToJsonFile(TArray<TSharedPtr<FOpenWorldTreeItem>>& InItems, FString InPath)
{
	FString OutString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutString);
	TSharedPtr<FJsonObject> Root = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> TreeJsonValues;
	for (int Index = 0; Index < InItems.Num(); Index++)
	{
		TSharedPtr<FJsonValueObject> NewJsonValue = MakeShared<FJsonValueObject>(TreeItemToJsonObject(InItems[Index]));
		TreeJsonValues.Add(NewJsonValue);
	}
	Root->SetArrayField("Tree", TreeJsonValues);
	if (FJsonSerializer::Serialize(Root.ToSharedRef(), Writer))
	{
		return FFileHelper::SaveStringToFile(OutString, *InPath);
	}
	
	return false;
}

bool FOpenWorldHelper::ReadFromJsonFile(TArray<TSharedPtr<FOpenWorldTreeItem>>& OutItems, FString InPath)
{
	if (!FPaths::FileExists(InPath))
	{
		return false;
	}
	
	FString StrContent;
	FFileHelper::LoadFileToString(StrContent, *InPath);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(StrContent);
	TSharedPtr<FJsonObject> Root;
	if (!FJsonSerializer::Deserialize(Reader, Root))
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* TreeJsonValues;
	if (!Root->TryGetArrayField(TEXT("Tree"), TreeJsonValues))
	{
		return false;
	}

	for (TSharedPtr<FJsonValue> JsonValueItem : *TreeJsonValues)
	{
		if(TSharedPtr<FJsonObject> NewJsonValue = JsonValueItem->AsObject())
		{
			TSharedPtr<FOpenWorldTreeItem> NewItem = JsonObjectToTreeItem(nullptr, NewJsonValue);
			OutItems.Add(NewItem);
		}
	}

	return true;
}

//Unchecked, Checked, Undetermined
ECheckBoxState GetCheckBoxStateFromInt(int Value)
{
	if (Value == 0)
	{
		return ECheckBoxState::Unchecked;
	}
	else if(Value == 1)
	{
		return ECheckBoxState::Checked;
	}
	else
	{
		return ECheckBoxState::Undetermined;
	}
}
int GetIntFromCheckBoxState(ECheckBoxState CheckBoxState)
{
	if (CheckBoxState == ECheckBoxState::Unchecked)
	{
		return 0;
	}
	else if(CheckBoxState == ECheckBoxState::Checked)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}
TSharedPtr<FJsonObject> FOpenWorldHelper::TreeItemToJsonObject(TSharedPtr<FOpenWorldTreeItem> InTreeItem)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("DisplayName", InTreeItem->DisplayName);
	JsonObject->SetStringField("IconColor", InTreeItem->IconColor.ToString());
	JsonObject->SetNumberField("IconType", InTreeItem->IconType);

	//Position
	TArray<TSharedPtr<FJsonValue>> IconPositionsJsonValues;
	for (int Index = 0; Index < InTreeItem->IconPostions.Num(); Index++)
	{
		TSharedPtr<FJsonValue> NewJsonValue = MakeShareable(new FJsonValueString(InTreeItem->IconPostions[Index].ToString()));
		IconPositionsJsonValues.Add(NewJsonValue);
	}
	JsonObject->SetArrayField("IconPositions", IconPositionsJsonValues);

	//Box
	TArray<TSharedPtr<FJsonValue>> IconBoxsJsonValues;
	for (int Index = 0; Index < InTreeItem->IconBoxs.Num(); Index++)
	{
		FString NewString = FString::Printf(TEXT("%s|%s"), *InTreeItem->IconBoxs[Index].Min.ToString(), *InTreeItem->IconBoxs[Index].Max.ToString());
		TSharedPtr<FJsonValue> NewJsonValue = MakeShareable(new FJsonValueString(NewString));
		IconBoxsJsonValues.Add(NewJsonValue);
	}
	JsonObject->SetArrayField("IconBoxs", IconBoxsJsonValues);
	
	JsonObject->SetNumberField("CheckBoxState", GetIntFromCheckBoxState(InTreeItem->GetCheckBoxState()));

	TArray<TSharedPtr<FJsonValue>> ChildrenJsonValues;
	for (int Index = 0; Index < InTreeItem->Children.Num(); Index++)
	{
		TSharedPtr<FJsonValueObject> NewJsonValue = MakeShared<FJsonValueObject>(TreeItemToJsonObject(InTreeItem->Children[Index]));
		ChildrenJsonValues.Add(NewJsonValue);
	}
	JsonObject->SetArrayField("Children", ChildrenJsonValues);

	return JsonObject;
}

TSharedPtr<FOpenWorldTreeItem> FOpenWorldHelper::JsonObjectToTreeItem(TSharedPtr<FOpenWorldTreeItem> InParentItem, TSharedPtr<FJsonObject> InJsonObject)
{
	if (!InJsonObject)
	{
		return nullptr;
	}
	
	TSharedPtr<FOpenWorldTreeItem> NewTreeItem = MakeShareable(new FOpenWorldTreeItem);
	NewTreeItem->DisplayName = InJsonObject->GetStringField("DisplayName");
	NewTreeItem->IconColor.InitFromString(InJsonObject->GetStringField("IconColor"));
	NewTreeItem->IconType = InJsonObject->GetNumberField("IconType");
	//Position
	TArray<TSharedPtr<FJsonValue>> IconPositionsJsonValues = InJsonObject->GetArrayField("IconPositions");
	for (int Index = 0; Index < IconPositionsJsonValues.Num(); Index++)
	{
		FVector NewPos;
		FString ItemString = IconPositionsJsonValues[Index]->AsString();
		if(NewPos.InitFromString(ItemString))
		{
			NewTreeItem->IconPostions.Add(NewPos);
		}
	}
	//Box
	TArray<TSharedPtr<FJsonValue>> IconBoxsJsonValues = InJsonObject->GetArrayField("IconBoxs");
	for (int Index = 0; Index < IconBoxsJsonValues.Num(); Index++)
	{
		FString ItemString = IconBoxsJsonValues[Index]->AsString();
		FString MinString, MaxString;
		if(ItemString.Split(TEXT("|"), &MinString, &MaxString))
		{
			FBox NewBox;
			NewBox.Min.InitFromString(MinString);
			NewBox.Max.InitFromString(MaxString);
			NewTreeItem->IconBoxs.Add(NewBox);
		}
	}
	NewTreeItem->CheckBoxState = GetCheckBoxStateFromInt(InJsonObject->GetNumberField("CheckBoxState"));

	TArray<TSharedPtr<FJsonValue>> ChildrenJsonValues = InJsonObject->GetArrayField("Children");
	for (int Index = 0; Index < ChildrenJsonValues.Num(); Index++)
	{
		TSharedPtr<FJsonObject> NewJsonObject = ChildrenJsonValues[Index]->AsObject();
		TSharedPtr<FOpenWorldTreeItem> NewChildItem = JsonObjectToTreeItem(NewTreeItem, NewJsonObject);
		NewChildItem->Parent = NewTreeItem;
		NewTreeItem->Children.Add(NewChildItem);
	}

	return NewTreeItem;
}

void FOpenWorldHelper::GetCheckedItems(TSharedPtr<FOpenWorldTreeItem> InItem, TArray<TSharedPtr<FOpenWorldTreeItem>>& InCheckedItems)
{
	for (auto& ChildItem : InItem->Children)
	{
		GetCheckedItems(ChildItem, InCheckedItems);
	}
	if (InItem->GetCheckBoxState() != ECheckBoxState::Unchecked)
	{
		TSharedPtr<FOpenWorldTreeItem> ThisItem = MakeShareable(new FOpenWorldTreeItem);
		ThisItem->DisplayName = InItem->DisplayName;
		ThisItem->IconColor = InItem->IconColor;
		ThisItem->IconType = InItem->IconType;
		ThisItem->IconPostions = InItem->IconPostions;
		ThisItem->IconBoxs = InItem->IconBoxs;
		ThisItem->CheckBoxState = InItem->CheckBoxState;
		InCheckedItems.Add(ThisItem);
	}
}

TSharedPtr<FOpenWorldTreeItem> FOpenWorldHelper::CreateFilterItem(TSharedPtr<FOpenWorldTreeItem> InItem, TSharedPtr<FOpenWorldTreeItem> InParentItem, FString InFilterString)
{
	TSharedPtr<FOpenWorldTreeItem> ThisItem = nullptr;
	if (InItem->Filter(InFilterString))
	{
		ThisItem = MakeShareable(new FOpenWorldTreeItem);
		ThisItem->DisplayName = InItem->DisplayName;
		ThisItem->IconColor = InItem->IconColor;
		ThisItem->IconType = InItem->IconType;
		ThisItem->IconPostions = InItem->IconPostions;
		ThisItem->IconBoxs = InItem->IconBoxs;
		ThisItem->CheckBoxState = InItem->CheckBoxState;
		if (InParentItem)
		{
			ThisItem->Parent = InParentItem;
			InParentItem->Children.Add(ThisItem);
		}
		for (auto& ChildItem : InItem->Children)
		{
			CreateFilterItem(ChildItem, ThisItem, InFilterString);
		}
	}
		
	return ThisItem;
}
bool FOpenWorldHelper::RefreshMapJson(UWorld* InWorld, FString InClassName)
{
	if (!InWorld)
		return false;

	TArray<TSharedPtr<FOpenWorldTreeItem>> Items;
	TSharedPtr<FOpenWorldTreeItem> NewItem1 = MakeShareable(new FOpenWorldTreeItem());
	Items.Add(NewItem1);
	NewItem1->DisplayName = "APostProcessVolume";
	TActorIterator<APostProcessVolume> Iter = TActorIterator<APostProcessVolume>(InWorld);
	for (Iter; Iter; ++Iter)
	{
		if (Iter)
		{
			TSharedPtr<FOpenWorldTreeItem> NewChildItem = MakeShareable(new FOpenWorldTreeItem());
			NewItem1->Children.Add(NewChildItem);
			NewChildItem->Parent = NewItem1;
			NewChildItem->DisplayName = Iter->GetName();
			NewChildItem->IconColor = FLinearColor::White;
			NewChildItem->IconType = 2;
			NewChildItem->IconPostions.Add(Iter->GetActorLocation());
			NewChildItem->IconBoxs.Add(Iter->GetComponentsBoundingBox(true, true));
		}
	}

	//Write to json
	FString JsonFilePath = FPaths::ProjectPluginsDir() + "UEOpenWorldDevPanel/Content/Maps/" + InWorld->GetMapName() + ".json";
	WriteToJsonFile(Items, JsonFilePath);
	FOpenWorldHelper::Get()->SetSelectJsonPath(JsonFilePath);
	FOpenWorldHelper::Get()->JsonPathChanged.Broadcast(JsonFilePath);
    			
	FString TipString = FString::Printf(TEXT("Saved to %s"), *JsonFilePath);
	FText TipText = FText::FromString(TipString);
	FMessageDialog::Open(EAppMsgType::Ok, TipText);

	return true;
}

void FOpenWorldHelper::SelectMapJson()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		// Default path to find stats
		const FString DefaultPath = FPaths::ProjectPluginsDir() + TEXT("UEOpenWorldDevPanel/Content/Maps");

		// File open dialog
		TArray<FString> Filenames;
		bool bOpened = DesktopPlatform->OpenFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			LOCTEXT("SelectMapJson", "SelectMapJson").ToString(),
			DefaultPath,
			TEXT(""),
			TEXT( "Map Json|*.json" ),
			EFileDialogFlags::None,
			Filenames
			);

		// If we chose a file
		if(bOpened && Filenames.Num() == 1)
		{
			FOpenWorldHelper::Get()->SetSelectJsonPath(Filenames[0]);
			FOpenWorldHelper::Get()->JsonPathChanged.Broadcast(Filenames[0]);
		}
	}
}
#undef LOCTEXT_NAMESPACE
