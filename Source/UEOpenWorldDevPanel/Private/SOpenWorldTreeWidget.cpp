#include "SOpenWorldTreeWidget.h"

#include "Widgets/Input/SSearchBox.h"

#define LOCTEXT_NAMESPACE "SOpenWorldTreeWidget"
void SOpenWorldTreeWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSearchBox)
			.HintText( LOCTEXT("SearchHintText", "Search") )
			.OnTextChanged(this, &SOpenWorldTreeWidget::OnFilterStringChanged)
			.DelayChangeNotificationsWhileTyping(true)
		]
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(TreeView, STreeView<TSharedPtr<FOpenWorldTreeItem>>)
			.ItemHeight(20.0f)
			.SelectionMode(ESelectionMode::Single)
			.TreeItemsSource(&FilterTreeItems)
			.OnGetChildren(this, &SOpenWorldTreeWidget::OnGetChildren)
			.OnGenerateRow(this, &SOpenWorldTreeWidget::OnGenerateRow)
			.OnExpansionChanged(this, &SOpenWorldTreeWidget::OnExpansionChanged)
		]
	];

	//FOpenWorldHelper::Test();
	//FOpenWorldHelper::ReadFromJsonFile(RootTreeItems, FOpenWorldHelper::GetJsonFilePath());
	RequestTreeRefresh();
}

void SOpenWorldTreeWidget::OnGetChildren(TSharedPtr<FOpenWorldTreeItem> InItem, TArray< TSharedPtr<FOpenWorldTreeItem> >& OutItems)
{
	OutItems = InItem->GetChildren();
}

TSharedRef<ITableRow> SOpenWorldTreeWidget::OnGenerateRow(TSharedPtr<FOpenWorldTreeItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return	SNew(STableRow< TSharedPtr<FOpenWorldTreeItem> >, OwnerTable)
	.Visibility_Lambda([this, InItem]()
	{
		return InItem->Filter(FilterString) ? EVisibility::Visible : EVisibility::Collapsed;
	})
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("Checkbox"))
			.IsChecked_Lambda([InItem]() 
			{
				return InItem->GetCheckBoxState();
			})
			.OnCheckStateChanged_Lambda([this, InItem](ECheckBoxState InCheckBoxState)
			{
				InItem->SetCheckBoxState(InCheckBoxState);
				this->CollectCheckedItems();
			})
		]
		+SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->DisplayName))
			.ColorAndOpacity(FColorList::LightGrey)			
		]
	];
}


void SOpenWorldTreeWidget::OnExpansionChanged(TSharedPtr<FOpenWorldTreeItem> TreeItem, bool bIsExpanded)
{
	TreeItem->IsShouldExpandItem = bIsExpanded;
}

void SOpenWorldTreeWidget::RequestTreeRefresh()
{
	FilterTreeItems.Empty();
	for (int Index = 0; Index < RootTreeItems.Num(); Index++)
	{
		TSharedPtr<FOpenWorldTreeItem> FilterItem = RootTreeItems[Index]->CreateFilterItem(nullptr, FilterString);
		if (FilterItem)
		{
			FilterTreeItems.Add(FilterItem);
		}
	}

	if (TreeView.IsValid())
	{
		TreeView->RequestTreeRefresh();
	}
}

void SOpenWorldTreeWidget::OnFilterStringChanged(const FText& InFilterString)
{
	//FilterString = InFilterString.ToString();
	//RequestTreeRefresh();

	FilterString = InFilterString.ToString();
	if (TreeView.IsValid())
	{
		TreeView->RequestTreeRefresh();
	}
}

void SOpenWorldTreeWidget::CollectCheckedItems()
{
	TArray<TSharedPtr<FOpenWorldTreeItem>> CheckedItems;
	for (auto& Item : FilterTreeItems)
	{
		Item->GetCheckedItems(CheckedItems);
	}
}
#undef LOCTEXT_NAMESPACE