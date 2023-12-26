// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExcelToXMLTool.h"
#include "ExcelToXMLToolStyle.h"
#include "ExcelToXMLToolCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "ExcelToXMLSubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorUtilityWidget.h"
#include "Styling/SlateStyleRegistry.h"


static const FName ExcelToXMLToolName("ExcelToXMLTool");

#define LOCTEXT_NAMESPACE "FExcelToXMLToolModule"

void FExcelToXMLToolModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("ExcelToXMLTool")->GetBaseDir();

	FExcelToXMLToolStyle::Initialize();
	FExcelToXMLToolStyle::ReloadTextures();

	FExcelToXMLToolCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FExcelToXMLToolCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FExcelToXMLToolModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FExcelToXMLToolModule::RegisterMenus));

	EditorUtilityWidgetPtr = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, TEXT("/ExcelToXMLTool/Widget/EUW_ExcelToXmlWidget"));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ExcelToXMLToolName, FOnSpawnTab::CreateUObject(EditorUtilityWidgetPtr, &UEditorUtilityWidgetBlueprint::SpawnEditorUITab))
		.SetDisplayName(LOCTEXT("FExcelToXMLToolTabTitle", "ExcelToXMLTool"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	
	/*FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ExcelToXMLToolName, FOnSpawnTab::CreateRaw(this, &FExcelToXMLToolModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FExcelToXMLToolTabTitle", "ExcelToXMLTool"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);*/
}

void FExcelToXMLToolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FExcelToXMLToolStyle::Shutdown();

	FExcelToXMLToolCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ExcelToXMLToolName);
}

TSharedRef<SDockTab> FExcelToXMLToolModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	EditorUtilityWidgetPtr = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, TEXT("/ExcelToXMLTool/Widget/EUW_ExcelToXmlWidget"));

	// 툴의 탭이 아웃라이너 패널, 디테일 패널과 같이 패널레이아웃 내부에서 놓이는게(탭끼리 이웃해서 놓이는게) 가능하려면 SDockTab의 TabRole이 ETabRole::NomadTab이 되어야 하는데 이를 설정할 방법을 찾아야한다.
	return EditorUtilityWidgetPtr->SpawnEditorUITab(SpawnTabArgs);

	//return SNew(SDockTab)
	//	.TabRole(ETabRole::PanelTab)
	//	[
	//		// Put your tab content here!
	//		SNew(SBox)
	//		.HAlign(HAlign_Center)
	//	.VAlign(VAlign_Center)
	//	[
	//		SNew(STextBlock)
	//	]
	//	];
}

void FExcelToXMLToolModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ExcelToXMLToolName);
}

void FExcelToXMLToolModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->RegisterMenu("MainFrame.MainMenu.Tools");
		FToolMenuSection& Section = Menu->AddSection("MyTools", LOCTEXT("MyToolsHeading", "MyTools"));
	}

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("MyTools");
			Section.AddMenuEntryWithCommandList
			(
				FExcelToXMLToolCommands::Get().OpenPluginWindow,
				PluginCommands,
				LOCTEXT("ExcelToXMLTitle", "ExcelToXMLTool"),
				LOCTEXT("ExcelToXMLTooltipText", "Convert Xlsx data table file to xml file."),
				FSlateIcon("ExcelToXMLToolStyle", "ExcelToXMLTool.Icon02")
			);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("MyTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FExcelToXMLToolCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}		
	}

	{
		////툴바에 버튼추가
		//UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		//{
		//	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("MyTools");
		//	{
		//		FToolMenuEntry& Entry = Section.AddEntry
		//		(
		//			FToolMenuEntry::InitToolBarButton
		//			(
		//				FExcelToXMLToolCommands::Get().OpenPluginWindow,
		//				LOCTEXT("ExcelToXMLTitle", "ExcelToXMLTool"),
		//				LOCTEXT("ExcelToXMLTooltipText", "Convert Xlsx data table file to xml file."),
		//				FSlateIcon("ExcelToXMLToolStyle", "ExcelToXMLTool.Icon02")
		//			)
		//		);
		//		Entry.SetCommandList(PluginCommands);
		//	}
		//}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FExcelToXMLToolModule, ExcelToXMLTool)