#pragma once
#include "Dom/JsonObject.h"


DECLARE_MULTICAST_DELEGATE_OneParam( FOnOpenWorldTreeChanged, FString );

struct FOpenWorldTreeItem : public TSharedFromThis<FOpenWorldTreeItem>
{
	FOpenWorldTreeItem()
	{
		IconType = 0;
		IconColor = FLinearColor::White;
		CheckBoxState = ECheckBoxState::Unchecked;
	}

	TArray<TSharedPtr<FOpenWorldTreeItem>>& GetChildren()
	{
		return Children;
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
	//Unchecked, Checked, Undetermined
	ECheckBoxState CheckBoxState;
};

class FOpenWorldHelper
{
public:
	static FOpenWorldHelper* Get()
	{
		if (!_Instance)
		{
			_Instance = new FOpenWorldHelper();
		}
		return _Instance;
	}
	void Broadcast(FString InTag)
	{
		TreeDataChanged.Broadcast(InTag);
	}
	FOnOpenWorldTreeChanged TreeDataChanged;

public:
	static FString GetJsonFilePath();
	static bool WriteToJsonFile(TArray<TSharedPtr<FOpenWorldTreeItem>>& InItems, FString InPath);
	static bool ReadFromJsonFile(TArray<TSharedPtr<FOpenWorldTreeItem>>& OutItems, FString InPath);
	static bool Test();

	static TSharedPtr<FJsonObject> TreeItemToJsonObject(TSharedPtr<FOpenWorldTreeItem> InTreeItem);
	static TSharedPtr<FOpenWorldTreeItem> JsonObjectToTreeItem(TSharedPtr<FOpenWorldTreeItem> InParentItem, TSharedPtr<FJsonObject> InJsonObject);
	static void GetCheckedItems(TSharedPtr<FOpenWorldTreeItem> InItem, TArray<TSharedPtr<FOpenWorldTreeItem>>& InCheckedItems);
	static TSharedPtr<FOpenWorldTreeItem> CreateFilterItem(TSharedPtr<FOpenWorldTreeItem> InThisItem, TSharedPtr<FOpenWorldTreeItem> InParentItem, FString InFilterString);

	
private:
	static FOpenWorldHelper* _Instance;
};

