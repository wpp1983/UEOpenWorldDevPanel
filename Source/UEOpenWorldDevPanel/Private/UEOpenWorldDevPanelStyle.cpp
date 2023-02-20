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

TSharedRef< FSlateStyleSet > FUEOpenWorldDevPanelStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("UEOpenWorldDevPanelStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("UEOpenWorldDevPanel")->GetBaseDir() / TEXT("Resources"));

	Style->Set("UEOpenWorldDevPanel.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

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
