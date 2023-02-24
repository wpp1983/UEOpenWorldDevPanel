#pragma once
#include "Dom/JsonObject.h"


struct FOpenWorldTreeItem : public TSharedFromThis<FOpenWorldTreeItem>
{
	FOpenWorldTreeItem()
	{
		IconType = 0;
		IconColor = FLinearColor::White;
		CheckBoxState = ECheckBoxState::Unchecked;
	}
	
	TSharedPtr<FJsonObject> ToJsonObject()
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		JsonObject->SetStringField("DisplayName", DisplayName);
		JsonObject->SetStringField("IconColor", IconColor.ToString());
		JsonObject->SetNumberField("IconType", IconType);
		
		TArray<TSharedPtr<FJsonValue>> IconPositionsJsonValues;
		for (int Index = 0; Index < IconPostions.Num(); Index++)
		{
			TSharedPtr<FJsonValue> NewJsonValue = MakeShareable(new FJsonValueString(IconPostions[Index].ToString()));
			IconPositionsJsonValues.Add(NewJsonValue);
		}
		JsonObject->SetArrayField("IconPositions", IconPositionsJsonValues);

		TArray<TSharedPtr<FJsonValue>> ChildrenJsonValues;
		for (int Index = 0; Index < Children.Num(); Index++)
		{
			TSharedPtr<FJsonValueObject> NewJsonValue = MakeShared<FJsonValueObject>(Children[Index]->ToJsonObject());
			ChildrenJsonValues.Add(NewJsonValue);
		}
		JsonObject->SetArrayField("Children", ChildrenJsonValues);

		return JsonObject;
	}

	bool FromJsonObject(TSharedPtr<FJsonObject> InJsonObject)
	{
		if (!InJsonObject)
		{
			return false;
		}

		DisplayName = InJsonObject->GetStringField("DisplayName");
		IconColor.InitFromString(InJsonObject->GetStringField("IconColor"));
		IconType = InJsonObject->GetNumberField("IconType");
		TArray<TSharedPtr<FJsonValue>> IconPositionsJsonValues = InJsonObject->GetArrayField("IconPositions");
		for (int Index = 0; Index < IconPositionsJsonValues.Num(); Index++)
		{
			FVector NewPos;
			FString ItemString = IconPositionsJsonValues[Index]->AsString();
			//TSharedPtr<FJsonValueString> Item = MakeShared<FJsonValueString>(IconPositionsJsonValues[Index]->AsString());
			NewPos.InitFromString(ItemString);
			IconPostions.Add(NewPos);
		}

		TArray<TSharedPtr<FJsonValue>> ChildrenJsonValues = InJsonObject->GetArrayField("Children");
		for (int Index = 0; Index < ChildrenJsonValues.Num(); Index++)
		{
			TSharedPtr<FJsonObject> NewJsonObject = ChildrenJsonValues[Index]->AsObject();
			TSharedPtr<FOpenWorldTreeItem> NewChildItem = MakeShareable(new FOpenWorldTreeItem);
			NewChildItem->FromJsonObject(NewJsonObject);
			NewChildItem->Parent = MakeShareable(this);
			Children.Add(NewChildItem);
		}

		return true;
	}

	TArray<TSharedPtr<FOpenWorldTreeItem>>& GetChildren()
	{
		return Children;
	}

	void GetCheckedItems(TArray<TSharedPtr<FOpenWorldTreeItem>>& InCheckedItems)
	{
		for (auto& Item : Children)
		{
			Item->GetCheckedItems(InCheckedItems);
		}
		if (GetCheckBoxState() != ECheckBoxState::Unchecked)
		{
			TSharedPtr<FOpenWorldTreeItem> ThisItem = MakeShareable(new FOpenWorldTreeItem);
			ThisItem->DisplayName = this->DisplayName;
			ThisItem->IconColor = this->IconColor;
			ThisItem->IconType = this->IconType;
			ThisItem->IconPostions = this->IconPostions;
			InCheckedItems.Add(ThisItem);
		}
	}

	bool Filter(FString InFilterString)
	{
		if (InFilterString.IsEmpty())
		{
			return true;
		}
		
		if (DisplayName.Contains(InFilterString))
		{
			return true;
		}

		for (auto& Item : Children)
		{
			if (Item->Filter(InFilterString))
			{
				return true;
			}
		}

		return false;
	}

	TSharedPtr<FOpenWorldTreeItem> CreateFilterItem(TSharedPtr<FOpenWorldTreeItem> InParentItem, FString InFilterString)
	{
		TSharedPtr<FOpenWorldTreeItem> ThisItem = nullptr;
		if (Filter(InFilterString))
		{
			ThisItem = MakeShareable(new FOpenWorldTreeItem);
			ThisItem->DisplayName = this->DisplayName;
			ThisItem->IconColor = this->IconColor;
			ThisItem->IconType = this->IconType;
			ThisItem->IconPostions = this->IconPostions;

			if (InParentItem)
			{
				ThisItem->Parent = InParentItem;
				InParentItem->Children.Add(ThisItem);
			}
			for (auto& Item : Children)
			{
				Item->CreateFilterItem(ThisItem, InFilterString);
			}
		}
		
		return ThisItem;
	}

	ECheckBoxState GetCheckBoxState()
	{
		ECheckBoxState Result = ECheckBoxState::Unchecked;
		if (Children.Num() == 0)
		{
			Result = CheckBoxState;
		}
		else
		{
			TArray<TSharedPtr<FOpenWorldTreeItem>> CheckedChildren = Children.FilterByPredicate([](TSharedPtr<FOpenWorldTreeItem> Item){
				return Item->GetCheckBoxState() != ECheckBoxState::Unchecked;
			});
			if (CheckedChildren.Num() == 0)
			{
				Result = ECheckBoxState::Unchecked;
			}
			else if (CheckedChildren.Num() == Children.Num())
			{
				Result = ECheckBoxState::Checked;
			}
			else
			{
				Result = ECheckBoxState::Undetermined;
			}
		}

		return Result;
	}

	void SetCheckBoxState(ECheckBoxState InCheckBoxState)
	{
		CheckBoxState = InCheckBoxState;
		for (auto& Item : Children)
		{
			Item->SetCheckBoxState(InCheckBoxState);
		}
	}

public:
	TSharedPtr<FOpenWorldTreeItem> Parent;
	TArray<TSharedPtr<FOpenWorldTreeItem>> Children;

	FString DisplayName;
	int IconType;
	FLinearColor IconColor;
	TArray<FVector> IconPostions;
	ECheckBoxState CheckBoxState;
	bool IsShouldExpandItem;
};

class FOpenWorldHelper
{
public:
	static FString GetJsonFilePath();
	static bool WriteToJsonFile(TArray<TSharedPtr<FOpenWorldTreeItem>>& InItems, FString InPath);
	static bool ReadFromJsonFile(TArray<TSharedPtr<FOpenWorldTreeItem>>& OutItems, FString InPath);

	static bool Test();
};

