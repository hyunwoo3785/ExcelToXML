// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExcelToXMLToolCommands.h"

#define LOCTEXT_NAMESPACE "FExcelToXMLToolModule"

void FExcelToXMLToolCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ExcelToXMLTool", "Bring up ExcelToXMLTool window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
