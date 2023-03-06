#include "UEOpenWorldDevPanel/Public/OpenWorldHelper.h"

#include "DesktopPlatformModule.h"
#include "EngineUtils.h"
#include "LevelEditorViewport.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SpotLight.h"
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
	JsonObject->SetNumberField("CheckBoxState", GetIntFromCheckBoxState(InTreeItem->GetCheckBoxState()));
	JsonObject->SetStringField("Name", InTreeItem->Name);
	JsonObject->SetNumberField("Type", (uint8)InTreeItem->Type);
	JsonObject->SetStringField("Postion", InTreeItem->Postion.ToString());
	//Properties
	TSharedPtr<FJsonObject> PropertiesJsonObject = MakeShareable(new FJsonObject());
	for (auto& PropertyItem : InTreeItem->Properties)
	{
		PropertiesJsonObject->SetStringField(PropertyItem.Key, PropertyItem.Value);
	}
	JsonObject->SetObjectField("Properties", PropertiesJsonObject);
	//Children
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
	NewTreeItem->CheckBoxState = GetCheckBoxStateFromInt(InJsonObject->GetNumberField("CheckBoxState"));
	NewTreeItem->Name = InJsonObject->GetStringField("Name");
	NewTreeItem->Type = (EIconType)InJsonObject->GetNumberField("Type");
	NewTreeItem->Postion.InitFromString(InJsonObject->GetStringField("Position"));
	//Properties
	TSharedPtr<FJsonObject> PropertiesJsonObject = InJsonObject->GetObjectField("Properties");
	for(auto& Item : PropertiesJsonObject->Values)
	{
		NewTreeItem->Properties.Add(Item.Key, Item.Value->AsString());
	}
	//Children
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
		ThisItem->CheckBoxState = InItem->CheckBoxState;
		ThisItem->Name = InItem->Name;
		ThisItem->Type = InItem->Type;
		ThisItem->Postion = InItem->Postion;
		ThisItem->Properties = InItem->Properties;
		InCheckedItems.Add(ThisItem);
	}
}

TSharedPtr<FOpenWorldTreeItem> FOpenWorldHelper::CreateFilterItem(TSharedPtr<FOpenWorldTreeItem> InItem, TSharedPtr<FOpenWorldTreeItem> InParentItem, FString InFilterString)
{
	TSharedPtr<FOpenWorldTreeItem> ThisItem = nullptr;
	if (InItem->Filter(InFilterString))
	{
		ThisItem = MakeShareable(new FOpenWorldTreeItem);
		ThisItem->CheckBoxState = InItem->CheckBoxState;
		ThisItem->Name = InItem->Name;
		ThisItem->Type = InItem->Type;
		ThisItem->Postion = InItem->Postion;
		ThisItem->Properties = InItem->Properties;
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

	TArray<APostProcessVolume*> PostProcessVolumes;
	TArray<APointLight*> PointLights;
	TArray<ASpotLight*> SpotLights;
	for (auto& ActorItem : InWorld->PersistentLevel->Actors)
	{
		if (APostProcessVolume* PostProcessVolume = Cast<APostProcessVolume>(ActorItem))
		{
			PostProcessVolumes.Add(PostProcessVolume);
		}
		else if(APointLight* PointLight = Cast<APointLight>(ActorItem))
		{
			PointLights.Add(PointLight);
		}
		else if(ASpotLight* SpotLight = Cast<ASpotLight>(ActorItem))
		{
			SpotLights.Add(SpotLight);
		}
	}
	TArray<TSharedPtr<FOpenWorldTreeItem>> Items;
	//PostProcessVolumes
	TSharedPtr<FOpenWorldTreeItem> PostProcessVolumesItem = MakeShareable(new FOpenWorldTreeItem());
	Items.Add(PostProcessVolumesItem);
	PostProcessVolumesItem->Name = "APostProcessVolume";
	for (auto& Item : PostProcessVolumes)
	{
		TSharedPtr<FOpenWorldTreeItem> NewChildItem = MakeShareable(new FOpenWorldTreeItem());
        PostProcessVolumesItem->Children.Add(NewChildItem);
        NewChildItem->Parent = PostProcessVolumesItem;
        NewChildItem->Name = Item->GetName();
        NewChildItem->Type = EIconType::APostProcessVolume;
        NewChildItem->Postion = Item->GetActorLocation();
        FBox ItemBox = Item->GetComponentsBoundingBox(true, true);
        FString ItemBoxString = FString::Printf(TEXT("%s|%s"), *ItemBox.Min.ToString(), *ItemBox.Max.ToString());
        NewChildItem->Properties.Add("BoundingBox", ItemBoxString);
	}
	//PointLights
	TSharedPtr<FOpenWorldTreeItem> PointLightsItem = MakeShareable(new FOpenWorldTreeItem());
	Items.Add(PointLightsItem);
	PointLightsItem->Name = "APointLight";
	for (auto& Item : PointLights)
	{
		TSharedPtr<FOpenWorldTreeItem> NewChildItem = MakeShareable(new FOpenWorldTreeItem());
		PointLightsItem->Children.Add(NewChildItem);
		NewChildItem->Parent = PointLightsItem;
		NewChildItem->Name = Item->GetName();
		NewChildItem->Type = EIconType::APointLight;
		NewChildItem->Postion = Item->GetActorLocation();
		NewChildItem->Properties.Add("LightColor", Item->GetLightColor().ToString());
		NewChildItem->Properties.Add("Intensity", FString::Printf(TEXT("%f"), Item->PointLightComponent->Intensity));
		NewChildItem->Properties.Add("AttenuationRadius", FString::Printf(TEXT("%f"), Item->PointLightComponent->AttenuationRadius));
	}
	//SpotLights
	TSharedPtr<FOpenWorldTreeItem> SpotLightsItem = MakeShareable(new FOpenWorldTreeItem());
	Items.Add(SpotLightsItem);
	SpotLightsItem->Name = "ASpotLight";
	for (auto& Item : SpotLights)
	{
		TSharedPtr<FOpenWorldTreeItem> NewChildItem = MakeShareable(new FOpenWorldTreeItem());
		SpotLightsItem->Children.Add(NewChildItem);
		NewChildItem->Parent = SpotLightsItem;
		NewChildItem->Name = Item->GetName();
		NewChildItem->Type = EIconType::ASpotLight;
		NewChildItem->Postion = Item->GetActorLocation();
		NewChildItem->Properties.Add("Rotation", Item->GetActorRotation().ToString());
		NewChildItem->Properties.Add("LightColor", Item->GetLightColor().ToString());
		NewChildItem->Properties.Add("Intensity", FString::Printf(TEXT("%f"), Item->SpotLightComponent->Intensity));
		NewChildItem->Properties.Add("AttenuationRadius", FString::Printf(TEXT("%f"), Item->SpotLightComponent->AttenuationRadius));
		NewChildItem->Properties.Add("InnerConeAngle", FString::Printf(TEXT("%f"), Item->SpotLightComponent->InnerConeAngle));
		NewChildItem->Properties.Add("OuterConeAngle", FString::Printf(TEXT("%f"), Item->SpotLightComponent->OuterConeAngle));
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
