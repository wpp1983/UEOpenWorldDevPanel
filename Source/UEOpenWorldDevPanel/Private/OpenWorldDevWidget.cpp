#include "OpenWorldDevWidget.h"

#include "OpenWorldDevTextureWidget.h"
#include "SOpenWorldTreeWidget.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "WorldPartition/WorldPartition.h"
#include "SOpenWorldTreeWidget.h"

SOpenWorldDevWidget::SOpenWorldDevWidget()
{
	
}

void SOpenWorldDevWidget::Construct(const FArguments& InArgs)
{
	SAssignNew(OpenWorldDevTreeWidget,SOpenWorldTreeWidget);
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
				SNew(SSplitter)
			    .Orientation(Orient_Horizontal)
				+SSplitter::Slot()
				.Value(0.3f)
				[
					OpenWorldDevTreeWidget.ToSharedRef()
				]
				+SSplitter::Slot()
				.Value(0.7f)
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

