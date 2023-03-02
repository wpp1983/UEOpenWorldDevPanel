// Copyright Epic Games, Inc. All Rights Reserved.

#include "UEOpenWorldDevPanelStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FUEOpenWorldDevPanelStyle::StyleInstance = nullptr;

void FUEOpenWorldDevPanelStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUEOpenWorldDevPanelStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FUEOpenWorldDevPanelStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UEOpenWorldDevPanelStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(20.0f, 20.0f);
const FVector2D Icon0x16(0.0f, 0.0f);

TSharedRef< FSlateStyleSet > FUEOpenWorldDevPanelStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("UEOpenWorldDevPanelStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("UEOpenWorldDevPanel")->GetBaseDir() / TEXT("Resources"));

	Style->Set("UEOpenWorldDevPanel.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("UEOpenWorldDevPanel.CollectPostProcessVolume", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("OpenWorldDevPannle.Monster", new IMAGE_BRUSH(TEXT("Icon128"), Icon20x20));
	Style->Set("OpenWorldDevPannle.Boss", new IMAGE_BRUSH(TEXT("BlueCycle"), Icon20x20));


	const FButtonStyle Button = FButtonStyle()
			.SetNormal(BOX_BRUSH(TEXT("Icon128"), Icon20x20))
			.SetHovered(BOX_BRUSH(TEXT("BlueCycle"), Icon20x20))
			.SetPressed(BOX_BRUSH(TEXT("BlueCycle"), Icon20x20));
			{
				Style->Set("OpenWorldDevPannle.Button",Button);
			}
			// .SetNormalPadding(FMargin(2, 2, 2, 2))
			// .SetPressedPadding(FMargin(2, 3, 2, 1))
			// .SetNormalForeground(InvertedForeground)
			// .SetPressedForeground(InvertedForeground)
			// .SetHoveredForeground(InvertedForeground)
			// .SetDisabledForeground(InvertedForeground);
	
	return Style;
}

void FUEOpenWorldDevPanelStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FUEOpenWorldDevPanelStyle::Get()
{
	return *StyleInstance;
}
