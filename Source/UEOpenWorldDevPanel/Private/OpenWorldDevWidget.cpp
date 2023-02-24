#include "OpenWorldDevWidget.h"

#include "OpenWorldDevTextureWidget.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "WorldPartition/WorldPartition.h"


SOpenWorldDevWidget::SOpenWorldDevWidget()
{
	
}

void SOpenWorldDevWidget::Construct(const FArguments& InArgs)
{

	SAssignNew(OpenWorldDevTextureWidget,SOpenWorldDevTextureWidget);
	ChildSlot
	[
		SNew(SOverlay)
		// Top status bar
		+SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			.FillHeight(1)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString("Capture World To 2DTextrue"))
					.OnClicked(OpenWorldDevTextureWidget.Get(),&SOpenWorldDevTextureWidget::CaptureWorldTo2D)
				]
				+SHorizontalBox::Slot()
				.FillWidth(1)
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					OpenWorldDevTextureWidget.ToSharedRef()
				]
			]
		]
	];


}
SOpenWorldDevWidget::~SOpenWorldDevWidget()
{
	
}

