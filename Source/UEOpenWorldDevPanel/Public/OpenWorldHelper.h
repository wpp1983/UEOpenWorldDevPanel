#pragma once
#include "Dom/JsonObject.h"


DECLARE_MULTICAST_DELEGATE_OneParam( FOnOpenWorldJsonPathChanged, FString );
DECLARE_MULTICAST_DELEGATE_OneParam( FOnOpenWorldJsonDataChanged, FString );

UENUM(BlueprintType)
enum class EIconType : uint8
{
	None,
	APostProcessVolume,
	APointLight,
	ASpotLight,
	Monster ,
	Boss ,
	ButtonIcon,
	Test2,
};

struct FOpenWorldTreeItem : public TSharedFromThis<FOpenWorldTreeItem>
{
	FOpenWorldTreeItem()
	{
		Type = EIconType::None;
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
		
		if (Name.Contains(InFilterString))
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
	
	bool TryGetBoundingBox(FBox& InOutBox)
	{
		if (Properties.Contains("BoundingBox"))
		{
			FString ItemString = Properties["BoundingBox"];
			FString MinString, MaxString;
			if(ItemString.Split(TEXT("|"), &MinString, &MaxString))
			{
				InOutBox.Min.InitFromString(MinString);
				InOutBox.Max.InitFromString(MaxString);
			}

			return true;
		}
		
		return false;
	}

	template <class T>
	bool TryGetPropertyByName(FString PropertyKey,T &Out)
	{
		if (Properties.Contains(PropertyKey))
		{
			FString ItemString = Properties[PropertyKey];
			Out.InitFromString(ItemString);
			return true;
		}
		return false;
	}

	// float double
	template <class T>
	bool TryGetNumericalValueByName(FString PropertyKey,T &Out)
	{
		if (Properties.Contains(PropertyKey))
		{
			FString ItemString = Properties[PropertyKey];
			Out = FCString::Atod(*ItemString);
			return true;
		}
		return false;
	}
public:
	TSharedPtr<FOpenWorldTreeItem> Parent;
	TArray<TSharedPtr<FOpenWorldTreeItem>> Children;

	ECheckBoxState CheckBoxState;
	FString Name;
	EIconType Type;
	FVector Position;
	TMap<FString, FString> Properties;
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

	FString GetSelectJsonPath();
	void SetSelectJsonPath(FString InJsonPath);

	FOnOpenWorldJsonPathChanged JsonPathChanged;
	FOnOpenWorldJsonDataChanged JsonDataChanged;
	FString SelectJsonPath;

public:
	static bool WriteToJsonFile(TArray<TSharedPtr<FOpenWorldTreeItem>>& InItems, FString InPath);
	static bool ReadFromJsonFile(TArray<TSharedPtr<FOpenWorldTreeItem>>& OutItems, FString InPath);

	static TSharedPtr<FJsonObject> TreeItemToJsonObject(TSharedPtr<FOpenWorldTreeItem> InTreeItem);
	static TSharedPtr<FOpenWorldTreeItem> JsonObjectToTreeItem(TSharedPtr<FOpenWorldTreeItem> InParentItem, TSharedPtr<FJsonObject> InJsonObject);
	static void GetCheckedItems(TSharedPtr<FOpenWorldTreeItem> InItem, TArray<TSharedPtr<FOpenWorldTreeItem>>& InCheckedItems);
	static TSharedPtr<FOpenWorldTreeItem> CreateFilterItem(TSharedPtr<FOpenWorldTreeItem> InThisItem, TSharedPtr<FOpenWorldTreeItem> InParentItem, FString InFilterString);

	static bool RefreshMapJson(UWorld* InWorld, FString InClassName);
	static void SelectMapJson();
	
private:
	static FOpenWorldHelper* _Instance;
};

