#include "UEOpenWorldDevPanel/Public/OpenWorldHelper.h"

#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "FOpenWorldHelper"

FOpenWorldHelper* FOpenWorldHelper::_Instance = nullptr;

FString FOpenWorldHelper::GetJsonFilePath()
{
	FString JsonFilePath = FPaths::ProjectPluginsDir() + "UEOpenWorldDevPanel/Tree.json";

	return JsonFilePath;
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

bool FOpenWorldHelper::Test()
{
	TArray<TSharedPtr<FOpenWorldTreeItem>> Items;
	{
		TSharedPtr<FOpenWorldTreeItem> NewItem1 = MakeShareable(new FOpenWorldTreeItem());
		Items.Add(NewItem1);
		NewItem1->DisplayName = "Parent1";
		NewItem1->IconColor = FLinearColor::Red;
		NewItem1->IconType = 1;
		{
			TSharedPtr<FOpenWorldTreeItem> NewChildItem1 = MakeShareable(new FOpenWorldTreeItem());
			NewItem1->Children.Add(NewChildItem1);
			NewChildItem1->Parent = NewItem1;
			NewChildItem1->DisplayName = "Child1";
			NewChildItem1->IconColor = FLinearColor::Green;
			NewChildItem1->IconType = 2;

			TSharedPtr<FOpenWorldTreeItem> NewChildItem2 = MakeShareable(new FOpenWorldTreeItem());
			NewItem1->Children.Add(NewChildItem2);
			NewChildItem2->Parent = NewItem1;
			NewChildItem2->DisplayName = "Child2";
			NewChildItem2->IconColor = FLinearColor::Blue;
			NewChildItem2->IconType = 3;
		}

		TSharedPtr<FOpenWorldTreeItem> NewItem2 = MakeShareable(new FOpenWorldTreeItem());
		Items.Add(NewItem2);
		NewItem2->DisplayName = "Parent2";
		NewItem2->IconColor = FLinearColor::White;
		NewItem2->IconType = 1;
		{
			TSharedPtr<FOpenWorldTreeItem> NewChildItem1 = MakeShareable(new FOpenWorldTreeItem());
			NewItem2->Children.Add(NewChildItem1);
			NewChildItem1->Parent = NewItem2;
			NewChildItem1->DisplayName = "Child1";
			NewChildItem1->IconColor = FLinearColor::Red;
			NewChildItem1->IconType = 2;

			TSharedPtr<FOpenWorldTreeItem> NewChildItem2 = MakeShareable(new FOpenWorldTreeItem());
			NewItem2->Children.Add(NewChildItem2);
			NewChildItem2->Parent = NewItem2;
			NewChildItem2->DisplayName = "Child2";
			NewChildItem2->IconColor = FLinearColor::Green;
			NewChildItem2->IconType = 3;

			TSharedPtr<FOpenWorldTreeItem> NewChildItem3 = MakeShareable(new FOpenWorldTreeItem());
			NewItem2->Children.Add(NewChildItem3);
			NewChildItem3->Parent = NewItem2;
			NewChildItem3->DisplayName = "Child3";
			NewChildItem3->IconColor = FLinearColor::Blue;
			NewChildItem3->IconType = 4;
		}	
	}

	FString JsonFilePath = GetJsonFilePath();
	WriteToJsonFile(Items, JsonFilePath);

	TArray<TSharedPtr<FOpenWorldTreeItem>> Items2;
	ReadFromJsonFile(Items2, JsonFilePath);

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
		
	TArray<TSharedPtr<FJsonValue>> IconPositionsJsonValues;
	for (int Index = 0; Index < InTreeItem->IconPostions.Num(); Index++)
	{
		TSharedPtr<FJsonValue> NewJsonValue = MakeShareable(new FJsonValueString(InTreeItem->IconPostions[Index].ToString()));
		IconPositionsJsonValues.Add(NewJsonValue);
	}
	JsonObject->SetArrayField("IconPositions", IconPositionsJsonValues);
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
#undef LOCTEXT_NAMESPACE
