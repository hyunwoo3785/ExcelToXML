// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ExcelToXMLToolStyle.h"

class FExcelToXMLToolCommands : public TCommands<FExcelToXMLToolCommands>
{
public:

	FExcelToXMLToolCommands()
		: TCommands<FExcelToXMLToolCommands>(TEXT("ExcelToXMLTool"), NSLOCTEXT("Contexts", "ExcelToXMLTool", "ExcelToXMLTool Plugin"), NAME_None, FExcelToXMLToolStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};