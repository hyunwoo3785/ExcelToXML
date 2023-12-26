#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ExcelToXMLSubSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogExcelToXMLSubsystem, Log, All)

namespace OpenXLSX 
{
	class XLWorkbook;
	class XLCell;
	class XLCellValueProxy;
}

UCLASS()
class UExcelToXMLSubSystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	void ParseAndGenerateToXlsxDataList(const FString& InFolderPath, TArray<TObjectPtr<class UXlsxData>>& OutXlsxDataList);
	bool ExportToXmlFile(const FString& InSaveFilePath, const TObjectPtr<const class UXlsxData> InXlsxData);
	bool ExportToXmlFile(const FString& InSaveFilePath, const TObjectPtr<const class UXlsxData> InXlsxData, const FString& InToExportStructureName);

	void SetWidgetBase(TObjectPtr<class UExcelToXmlWidgetBase> WidgetBase);
	TObjectPtr<class UExcelToXmlWidgetBase> GetWidgetBase();

private:
	bool Parse(class UXlsxData* OutXlsxData);

	void ParseSimpleTypeTable(class OpenXLSX::XLWorkbook& InWorkbook, TObjectPtr<class UXlsxData> OutXlsxData);
	void ParseIndexSheet(class OpenXLSX::XLWorkbook& InWorkbook, TObjectPtr<class UXlsxData> OutXlsxData);
	void ParseDataSheet(class OpenXLSX::XLWorkbook& InWorkbook, TObjectPtr<class UXlsxData> OutXlsxData, struct FXlsxStruct& OutStructData);

	FString CellValueToFString(const OpenXLSX::XLCellValueProxy& CellValue);

private:
	//class TSubclassOf<class EditorUtilityWidget> EditorUtilityWidgetClass;
	TObjectPtr<class UExcelToXmlWidgetBase> EditorUtilityWidget;
};