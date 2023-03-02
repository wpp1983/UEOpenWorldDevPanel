// Copyright Epic Games, Inc. All Rights Reserved.

#include "UEOpenWorldDevPanel.h"
#include "UEOpenWorldDevPanelStyle.h"
#include "UEOpenWorldDevPanelCommands.h"
#include "LevelEditorViewport.h"
#include "OpenWorldDevWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "OpenWorldHelper.h"
static const FName UEOpenWorldDevPanelTabName("UEOpenWorldDevPanel");

#define LOCTEXT_NAMESPACE "FUEOpenWorldDevPanelModule"

void FUEOpenWorldDevPanelModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FUEOpenWorldDevPanelStyle::Initialize();
	FUEOpenWorldDevPanelStyle::ReloadTextures();

	FUEOpenWorldDevPanelCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUEOpenWorldDevPanelCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUEOpenWorldDevPanelModule::OnOpenPluginWindow),
		FCanExecuteAction());

	PluginCommands->MapAction(
		FUEOpenWorldDevPanelCommands::Get().CollectPostProcessVolume,
		FExecuteAction::CreateRaw(this, &FUEOpenWorldDevPanelModule::OnCollectPostProcessVolume),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUEOpenWorldDevPanelModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UEOpenWorldDevPanelTabName, FOnSpawnTab::CreateRaw(this, &FUEOpenWorldDevPanelModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FUEOpenWorldDevPanelTabTitle", "UEOpenWorldDevPanel"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FUEOpenWorldDevPanelModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUEOpenWorldDevPanelStyle::Shutdown();

	FUEOpenWorldDevPanelCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UEOpenWorldDevPanelTabName);
}

TSharedRef<SDockTab> FUEOpenWorldDevPanelModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FUEOpenWorldDevPanelModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("UEOpenWorldDevPanel.cpp"))
		);
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SOpenWorldDevWidget)
			]
		];
}

void FUEOpenWorldDevPanelModule::OnOpenPluginWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UEOpenWorldDevPanelTabName);
}

void FUEOpenWorldDevPanelModule::OnCollectPostProcessVolume()
{
	if (GCurrentLevelEditingViewportClient)
	{
		UWorld* CurrentWorld = GCurrentLevelEditingViewportClient->GetWorld();
		FOpenWorldHelper::RefreshMapJson(CurrentWorld, "APostProcessVolume");
	}
}

void FUEOpenWorldDevPanelModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FUEOpenWorldDevPanelCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& OpenPluginEntry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUEOpenWorldDevPanelCommands::Get().OpenPluginWindow));
				OpenPluginEntry.SetCommandList(PluginCommands);

				FToolMenuEntry& CollectEntry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUEOpenWorldDevPanelCommands::Get().CollectPostProcessVolume));
				CollectEntry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUEOpenWorldDevPanelModule, UEOpenWorldDevPanel)