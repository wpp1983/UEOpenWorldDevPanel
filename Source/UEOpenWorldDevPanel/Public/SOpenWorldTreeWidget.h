#pragma once
#include "OpenWorldHelper.h"

class SOpenWorldTreeWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SOpenWorldTreeWidget){}
	SLATE_END_ARGS()
public:
	~SOpenWorldTreeWidget();
	void Construct(const FArguments& InArgs);
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FOpenWorldTreeItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TSharedPtr<FOpenWorldTreeItem> InItem, TArray< TSharedPtr<FOpenWorldTreeItem> >& OutItems);

	void RequestTreeRefresh();
	void OnFilterStringChanged(const FText& InFilterString);
	void CollectCheckedItems();
	void OnTreeDataChanged(FString InTag);

public:
	TSharedPtr<STreeView<TSharedPtr<FOpenWorldTreeItem>>> TreeView;
	TArray<TSharedPtr<FOpenWorldTreeItem>> RootTreeItems;
	FString FilterString;
};
