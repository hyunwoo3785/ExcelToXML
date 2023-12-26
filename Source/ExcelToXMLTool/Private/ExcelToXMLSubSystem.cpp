#include "ExcelToXMLSubSystem.h"
#include "ExcelToXMLTool.h"
#include "OpenXLSX.hpp"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Materials/Material.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "HAL/FileManager.h"
#include "ExcelToXmlWidgetBase.h"
#include "XlsxTableObject.h"
#include "tinyxml2.h"

#define LOCTEXT_NAMESPACE "ExcelToXMLSubsystem"

DEFINE_LOG_CATEGORY(LogExcelToXMLSubsystem)

#define SIMPLE_TABLE_PARSING_STARTED_ROW 2
#define STRUCT_DELIMITER L'%'
#define ENUM_DELIMITER L'&'

void UExcelToXMLSubSystem::ParseAndGenerateToXlsxDataList(const FString& FolderPath, TArray<TObjectPtr<UXlsxData>>& XlsxDataList)
{
    TArray<FString> Filelist;
    IFileManager::Get().FindFiles(Filelist, *FolderPath, TEXT(".xlsx"));

    if (Filelist.Num() == 0)
    {
        return;
    }

    ensure(EditorUtilityWidget);

    XlsxDataList.Empty(Filelist.Num());

    // 외부에서 엑셀프로그램에 의해 열린 파일이 있을경우 폴더내에 ~$Filename.xlsx 파일이 있기 때문에 제거해준다.
    while (FString* FindItem = Filelist.FindByPredicate([](const FString& Filename)
        {
            return Filename.Contains(TEXT("~$"));
        })) 
    {
        Filelist.Remove(FString(*FindItem));
    }
    
    for (const FString& Filename : Filelist)
    {
        FString PureFilename = FPaths::GetBaseFilename(Filename);
        UXlsxData* NewXlsxData = NewObject<UXlsxData>(EditorUtilityWidget, FName(*PureFilename));
        XlsxDataList.Add(NewXlsxData);

        NewXlsxData->Filename = Filename;
        NewXlsxData->Filepath = FolderPath;

        Parse(NewXlsxData);
    }
}

bool UExcelToXMLSubSystem::Parse(UXlsxData* XlsxData)
{
	using namespace OpenXLSX;
	using namespace std;

    ensure(XlsxData);

    const FString FullPath = FPaths::Combine(*XlsxData->Filepath, *XlsxData->Filename);

    if (!IFileManager::Get().FileExists(*FullPath))
    {
        UE_LOG(LogExcelToXMLSubsystem, Warning, TEXT("Does not exist file : %s"), *FullPath);
        return false;
    }

    XLDocument Document;
    Document.open(TCHAR_TO_UTF8(*FullPath));

    XLWorkbook Workbook = Document.workbook();

    unsigned int SheetCount = Workbook.worksheetCount();

    vector<string> SheetNames = Workbook.worksheetNames();

    if (SheetCount == 0)
    {
        Document.close();

        return false;
    }
    
    XLWorksheet FirstSheet = Workbook.sheet(1);

    FString FirstSheetName(FirstSheet.name().c_str());

    if (SheetCount == 1)
    {
        FString PureFilename = FPaths::GetBaseFilename(FullPath);

        if (!PureFilename.Equals(FirstSheetName, ESearchCase::IgnoreCase))
        {
            UE_LOG(LogExcelToXMLSubsystem, Warning, TEXT("단순타입 테이블파일(%s)의 첫 번째 시트의 이름(%s)이 파일의 이름과 같지 않습니다."), *XlsxData->Filename, *FirstSheetName);
            Document.close();

            return false;
        }
        
        XlsxData->Type = EParsingFileType::SimpleTable;
        ParseSimpleTypeTable(Workbook, XlsxData);
    }
    else
    {
        if (!FirstSheetName.Equals(TEXT("Index"), ESearchCase::IgnoreCase))
        {
            UE_LOG(LogExcelToXMLSubsystem, Warning, TEXT("복합타입 테이블파일의 첫 번째 시트의 이름(%s)이 'Index'가 아닙니다."), *FirstSheetName);
            Document.close();

            return false;
        }

        XlsxData->Type = EParsingFileType::ComplexTable;
        ParseIndexSheet(Workbook, XlsxData);
    }

    Document.close();

    return true;
}

void UExcelToXMLSubSystem::ParseSimpleTypeTable(OpenXLSX::XLWorkbook& InWorkbook, TObjectPtr<UXlsxData> OutXlsxData)
{
    using namespace OpenXLSX;
    using namespace std;
}

void UExcelToXMLSubSystem::ParseIndexSheet(OpenXLSX::XLWorkbook& InWorkbook, TObjectPtr<UXlsxData> OutXlsxData)
{
    using namespace OpenXLSX;
    using namespace std;

    XLWorksheet IndexSheet = InWorkbook.sheet(1);

    XLCellRange CellRange = IndexSheet.range();

    // Find cell of the structure or enum
    for (uint32_t rowIndex = 1; rowIndex <= CellRange.numRows(); rowIndex++)
    {
        XLCell Cell = IndexSheet.cell(rowIndex, 1);

        FString StringValue(Cell.value().get<string>().c_str());

        if (StringValue.IsEmpty())
        {
            continue;
        }
        TCHAR FirstCharacter = StringValue[0];
        if (FirstCharacter == STRUCT_DELIMITER)
        {
            FXlsxStruct StructData;
            StructData.Name = StringValue.RightChop(1);

            // Find empty cell index, meaning last index of this structure's row index
            uint32_t structureDefineEndedIndex = rowIndex;

            while (structureDefineEndedIndex <= CellRange.numRows())
            {
                string CurrentValue = IndexSheet.cell(structureDefineEndedIndex, 2).value();
                if (IndexSheet.cell(++structureDefineEndedIndex, 2).value().type() == XLValueType::Empty)
                {
                    break;
                }                
            }
            // Add field and type
            uint32_t fieldStartRowIndex = rowIndex + 1;
            StructData.FieldList.Empty(structureDefineEndedIndex - fieldStartRowIndex);

            for (fieldStartRowIndex; fieldStartRowIndex < structureDefineEndedIndex; fieldStartRowIndex++)
            {
                string fieldName = IndexSheet.cell(fieldStartRowIndex, 2).value();
                string fieldType = IndexSheet.cell(fieldStartRowIndex, 3).value();

                TPair<FString, FString> FieldPair;
                StructData.FieldList.Add(MakeTuple(FString(fieldName.c_str()), FString(fieldType.c_str())));
            }
            // 구조체 이름 시트의 데이터 파싱
            ParseDataSheet(InWorkbook, OutXlsxData, StructData);

            OutXlsxData->StructList.Add(StructData);
        }

        // 열거형 타입 인덱스 시트에서 파싱
        if (FirstCharacter == ENUM_DELIMITER)
        {
            FXlsxEnum EnumData;
            EnumData.Name = StringValue.RightChop(1);

            // Find empty cell index, meaningwhile last index of this Enum's row index
            uint32_t toFindEmtpyValueIndex = rowIndex;

            while (toFindEmtpyValueIndex <= CellRange.numRows())
            {
                if (IndexSheet.cell(++toFindEmtpyValueIndex, 2).value().type() == XLValueType::Empty)
                {
                    break;
                }
            }

            // Add field and type
            uint32_t FieldStartedRowIndex = rowIndex + 1;
            EnumData.FieldList.Empty(toFindEmtpyValueIndex - FieldStartedRowIndex);

            for (FieldStartedRowIndex; FieldStartedRowIndex < toFindEmtpyValueIndex; FieldStartedRowIndex++)
            {
                string FieldName = IndexSheet.cell(FieldStartedRowIndex, 2).value();
                uint32 EnumValue = IndexSheet.cell(FieldStartedRowIndex, 3).value();

                TPair<FString, uint32> FieldPair;
                EnumData.FieldList.Add(MakeTuple(FString(FieldName.c_str()), EnumValue));
            }

            OutXlsxData->EnumList.Add(EnumData);
        }
    }
}

void UExcelToXMLSubSystem::ParseDataSheet(OpenXLSX::XLWorkbook& InWorkbook, TObjectPtr<UXlsxData> OutXlsxData, FXlsxStruct& OutStructData)
{
    using namespace OpenXLSX;
    using namespace std;

    ensure(InWorkbook.worksheetExists(TCHAR_TO_UTF8(*OutStructData.Name)));

    XLWorksheet DataSheet = InWorkbook.sheet(TCHAR_TO_UTF8(*OutStructData.Name));
    XLCellRange CellRange = DataSheet.range();
    bool bFoundStartIndex = false;
    const uint32_t StartIndex = 1;
    TArray<TPair<FString, uint16>> FieldColumnIndexList;

    for (uint32_t DelimiterRowIndex = StartIndex; DelimiterRowIndex <= CellRange.numRows(); DelimiterRowIndex++)
    {
        XLCell Cell = DataSheet.cell(DelimiterRowIndex, 1); //1열에 시트의 테이블 시작을 구분하는 셀이 있다.

        FString RowCellDataInFirstColumn(Cell.value().get<string>().c_str());

        if (RowCellDataInFirstColumn.IsEmpty())
        {
            continue;
        }

        FString DelimiterLiteral(OutStructData.Name);
        DelimiterLiteral.InsertAt(0, TEXT('#'));
        DelimiterLiteral.Append(TEXT("[{}]"));

        if (RowCellDataInFirstColumn.Equals(DelimiterLiteral)) // #StructureName[{}] 라는 값이 들어있는 셀을 찾음
        {
            bFoundStartIndex = true;

            FieldColumnIndexList.Empty(CellRange.numColumns());
            for (uint32_t FiledColumnIndex = 2; FiledColumnIndex <= CellRange.numColumns(); FiledColumnIndex++) //#StructureName[{}] 오른쪽 셀들로 부터 필드와 필드의 Column 인덱스를 페어로 저장한다.
            {
                XLCell FieldCell = DataSheet.cell(DelimiterRowIndex, FiledColumnIndex);
                FString FieldCellValue(FieldCell.value().get<string>().c_str());

                if (OutStructData.FieldList.ContainsByPredicate([=](const TPair<FString, FString>& Field)
                    {
                        return Field.Key.Equals(FieldCellValue) ? true : false;
                    }))
                {
                    uint16 FieldColumnIndex = FieldCell.cellReference().column();
                    FieldColumnIndexList.Emplace(FieldCellValue, FieldColumnIndex);
                }
            }

            ensure(OutStructData.FieldList.Num() == FieldColumnIndexList.Num());

            // 테이블 데이터의 유효범위 설정
            // 테이블 데이터에서 값이 공백이 나올경우 데이터의 행의 끝을 의미한다.
            // 간혹 필드의 데이터 셀에 빈 데이터가 들어가있는 경우가 보이는데 첫번째 필드에서도 빈데이터가 들어갈 경우가 있는지 확인 필요
            uint32_t LastRowEmptyIndex = DelimiterRowIndex;
            while (LastRowEmptyIndex <= CellRange.numRows())
            {
                if (DataSheet.cell(++LastRowEmptyIndex, 2).value().type() == OpenXLSX::XLValueType::Empty)
				{
                    break;
                }
            }
            
            for (uint32_t DataRowIndex = DelimiterRowIndex + 1; DataRowIndex < LastRowEmptyIndex; DataRowIndex++)
            {
                TArray<FString> RowDataList;
                RowDataList.Empty(FieldColumnIndexList.Num());

                for (const TPair<FString, uint16> Pair : FieldColumnIndexList)
                {
                    XLCell FieldDataCell = DataSheet.cell(DataRowIndex, Pair.Value);
                    RowDataList.Emplace(CellValueToFString(FieldDataCell.value()));
                }
                OutStructData.SheetData.Emplace(RowDataList);
            }
        }
    }

    ensure(bFoundStartIndex);
}


bool UExcelToXMLSubSystem::ExportToXmlFile(const FString& InSaveFilepath, const TObjectPtr<const UXlsxData> InXlsxData)
{
    using namespace tinyxml2;

    if (!InXlsxData && InXlsxData->StructList.Num() == 0)
    {
        return false;
    }

    bool bExistData = false;
    for (const FXlsxStruct& Struct : InXlsxData->StructList)
    {
        if (Struct.SheetData.Num())
        {
            bExistData = true;
            break;
        }
    }
    if (!bExistData)
    {
        return false;
    }

	XMLDocument Document;

    XMLDeclaration* Declaration = Document.NewDeclaration();
    XMLElement* RootElement = Document.NewElement(TCHAR_TO_UTF8(*FPaths::GetBaseFilename(InXlsxData->Filename)));

    Document.LinkEndChild(Declaration);
    Document.LinkEndChild(RootElement);


    for (const FXlsxStruct& Struct : InXlsxData->StructList)
    {
        if (Struct.SheetData.Num() == 0)
        {
            continue;
        }

        for (const TArray<FString>& SheetData : Struct.SheetData)
        {
            XMLElement* StructElement = Document.NewElement(TCHAR_TO_UTF8(*Struct.Name));
            RootElement->LinkEndChild(StructElement);

            for (uint16 i = 0; i < Struct.FieldList.Num(); i++)
            {
                FString ValueType = Struct.FieldList[i].Value;
                if (ValueType.Equals(TEXT("bool"), ESearchCase::IgnoreCase))
                {
                    StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct.FieldList[i].Key), SheetData[i].Equals(TEXT("true"), ESearchCase::IgnoreCase) ? true : false);
                }
                else if (ValueType.Equals(TEXT("int"), ESearchCase::IgnoreCase))
                {
                    StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct.FieldList[i].Key), FCString::Atoi(*SheetData[i]));
                }
                else if (ValueType.Equals(TEXT("float"), ESearchCase::IgnoreCase))
                {
                    StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct.FieldList[i].Key), FCString::Atof(*SheetData[i]));
                }
                else if (ValueType.Equals(TEXT("double"), ESearchCase::IgnoreCase))
                {
                    StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct.FieldList[i].Key), FCString::Atod(*SheetData[i]));
                }
                else
                {
                    StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct.FieldList[i].Key), TCHAR_TO_UTF8(*SheetData[i]));
                }
            }
        }
    }

    XMLError Error = Document.SaveFile(TCHAR_TO_UTF8(*InSaveFilepath));
    if (Error != tinyxml2::XML_SUCCESS)
    {
		/*switch (Error)
		{
			case XMLError::
		}*/
        UE_LOG(LogExcelToXMLSubsystem, Error, TEXT("Does not exist %d"), Error);

        return false;
    }
    else
    {
        return true;
    }
}

bool UExcelToXMLSubSystem::ExportToXmlFile(const FString& SaveFilePath, const TObjectPtr<const UXlsxData> InXlsxData, const FString& InToExportStructureName)
{
    using namespace tinyxml2;

    FString SavePath = SaveFilePath;

    if (!InXlsxData && InXlsxData->StructList.Num() == 0)
    {
        return false;
    }

    const FXlsxStruct* Struct = InXlsxData->StructList.FindByKey(InToExportStructureName);
    if (Struct == nullptr)
    {
        return false;
    }

    XMLDocument Document;

    XMLDeclaration* Declaration = Document.NewDeclaration();
    XMLElement* RootElement = Document.NewElement(TCHAR_TO_UTF8(*FPaths::GetBaseFilename(Struct->Name)));

    Document.LinkEndChild(Declaration);
    Document.LinkEndChild(RootElement);

	if (Struct->SheetData.Num() == 0)
	{
		return false;
	}

	for (const TArray<FString>& SheetData : Struct->SheetData)
	{
		XMLElement* StructElement = Document.NewElement("");
		RootElement->LinkEndChild(StructElement);

		for (uint16 i = 0; i < Struct->FieldList.Num(); i++)
		{
			FString ValueType = Struct->FieldList[i].Value;
			if (ValueType.Equals(TEXT("bool"), ESearchCase::IgnoreCase))
			{
				StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct->FieldList[i].Key), SheetData[i].Equals(TEXT("true"), ESearchCase::IgnoreCase) ? true : false);
			}
			else if (ValueType.Equals(TEXT("int"), ESearchCase::IgnoreCase))
			{
				StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct->FieldList[i].Key), FCString::Atoi(*SheetData[i]));
			}
			else if (ValueType.Equals(TEXT("float"), ESearchCase::IgnoreCase))
			{
				StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct->FieldList[i].Key), FCString::Atof(*SheetData[i]));
			}
			else if (ValueType.Equals(TEXT("double"), ESearchCase::IgnoreCase))
			{
				StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct->FieldList[i].Key), FCString::Atod(*SheetData[i]));
			}
			else
			{
				StructElement->SetAttribute(TCHAR_TO_UTF8(*Struct->FieldList[i].Key), TCHAR_TO_UTF8(*SheetData[i]));
			}
		}
	}

    XMLError SaveError = Document.SaveFile(TCHAR_TO_UTF8(*SavePath));
    if (SaveError != tinyxml2::XML_SUCCESS)
    {
        /*switch (Error)
        {
            case XMLError::
        }*/
        UE_LOG(LogExcelToXMLSubsystem, Error, TEXT("SaveError code : %d"), SaveError);

        return false;
    }
    else
    {
        return true;
    }
}

FString UExcelToXMLSubSystem::CellValueToFString(const OpenXLSX::XLCellValueProxy& CellValue)
{
    OpenXLSX::XLValueType Type = CellValue.type();

	FString Value;
	switch (Type)
	{
	case OpenXLSX::XLValueType::Boolean:
		Value = CellValue.get<bool>() ? TEXT("true") : TEXT("false");
		break;
	case OpenXLSX::XLValueType::String:
	{
		Value = UTF8_TO_TCHAR(CellValue.get<std::string>().c_str());
	}
	break;
	case OpenXLSX::XLValueType::Integer:
		Value = FString::Printf(TEXT("%d"), CellValue.get<int64_t>());
		break;
	case OpenXLSX::XLValueType::Float:
		Value = FString::Printf(TEXT("%f"), CellValue.get<double>());
		break;
	default:
		break;
	}
    return Value;
}

void UExcelToXMLSubSystem::SetWidgetBase(TObjectPtr<UExcelToXmlWidgetBase> WidgetBase)
{
    EditorUtilityWidget = WidgetBase;
}

TObjectPtr<class UExcelToXmlWidgetBase> UExcelToXMLSubSystem::GetWidgetBase()
{
    return EditorUtilityWidget;
}


#undef LOCTEXT_NAMESPACE