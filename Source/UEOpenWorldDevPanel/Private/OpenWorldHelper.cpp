#include "UEOpenWorldDevPanel/Public/OpenWorldHelper.h"

#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "FOpenWorldHelper"

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
		TSharedPtr<FJsonValueObject> NewJsonValue = MakeShared<FJsonValueObject>(InItems[Index]->ToJsonObject());
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
			TSharedPtr<FOpenWorldTreeItem> NewItem = MakeShareable(new FOpenWorldTreeItem());
			NewItem->FromJsonObject(NewJsonValue);
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
#undef LOCTEXT_NAMESPACE
