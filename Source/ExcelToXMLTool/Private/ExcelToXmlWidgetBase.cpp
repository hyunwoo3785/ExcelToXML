// Fill out your copyright notice in the Description page of Project Settings.


#include "ExcelToXmlWidgetBase.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "Runtime/UMG/Public/Components/EditableTextBox.h"
#include "Runtime/UMG/Public/Components/ListView.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "XlsxTableObject.h"
#include "ExcelToXMLSubSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

#define LOCTEXT_NAMESPACE "UExcelToXmlWidgetBase"

DEFINE_LOG_CATEGORY(LogExcelToXmlTool)
UExcelToXmlWidgetBase::UExcelToXmlWidgetBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    if (UExcelToXmlWidgetBase::XlsxEntryWidgetClass_BP || UExcelToXmlWidgetBase::TableEntryWidgetClass_BP || UExcelToXmlWidgetBase::FieldEntryWidgetClass_BP)
    {
        return;
    }

    static ConstructorHelpers::FClassFinder<UUserWidget> XlsxEntryWidgetClassFinder(TEXT("WidgetBlueprint'/ExcelToXMLTool/Widget/W_XlsxEntryWidget.W_XlsxEntryWidget_C'"));
    static ConstructorHelpers::FClassFinder<UUserWidget> TableEntryWidgetClassFinder(TEXT("WidgetBlueprint'/ExcelToXMLTool/Widget/W_TableEntryWidget.W_TableEntryWidget_C'"));
    static ConstructorHelpers::FClassFinder<UUserWidget> FieldEntryWidgetClassFinder(TEXT("WidgetBlueprint'/ExcelToXMLTool/Widget/W_FieldEntryWidget.W_FieldEntryWidget_C'"));

    if (XlsxEntryWidgetClassFinder.Succeeded())
    {
        UExcelToXmlWidgetBase::XlsxEntryWidgetClass_BP = XlsxEntryWidgetClassFinder.Class;
    }
    if (TableEntryWidgetClassFinder.Succeeded())
    {
        UExcelToXmlWidgetBase::TableEntryWidgetClass_BP = TableEntryWidgetClassFinder.Class;
    }
    if (FieldEntryWidgetClassFinder.Succeeded())
    {
        UExcelToXmlWidgetBase::FieldEntryWidgetClass_BP = FieldEntryWidgetClassFinder.Class;
    }
    bAutoRunDefaultAction = false;
}

void UExcelToXmlWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

    ImportPathButton->OnClicked.AddDynamic(this, &UExcelToXmlWidgetBase::OnImportPathButtonClicked);
    ExportPathForClientButton->OnClicked.AddDynamic(this, &UExcelToXmlWidgetBase::OnExportPathForClientButtonClicked);
    ExportPathForServerButton->OnClicked.AddDynamic(this, &UExcelToXmlWidgetBase::OnExportPathForServerButtonClicked);
    RefreshButton->OnClicked.AddDynamic(this, &UExcelToXmlWidgetBase::OnRefreshButtonClicked);
    XmlExportButton->OnClicked.AddDynamic(this, &UExcelToXmlWidgetBase::OnXmlExportButtonClicked);

    ImportPathTextBox->SetText(FText::FromString(ImportPath));
    ExportPathForClientTextBox->SetText(FText::FromString(ExportPathForClient));
    ExportPathForServerTextBox->SetText(FText::FromString(ExportPathForServer));

    Subsystem = GEditor->GetEditorSubsystem<UExcelToXMLSubSystem>();
    ensure(Subsystem);

    Subsystem->SetWidgetBase(this);
    Subsystem->ParseAndGenerateToXlsxDataList(ImportPath, XlsxDataList);

    for (UXlsxData* Item : XlsxDataList)
    {
        XlsxListView->AddItem(Item);
    }

    XmlExportButton->SetIsEnabled(false);
}

void UExcelToXmlWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

#define IMPLEMENT_ON_PATH_BUTTON_CLICKED(PathPropertyName) \
 void UExcelToXmlWidgetBase::On##PathPropertyName##ButtonClicked() \
{ \
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get(); \
    if (DesktopPlatform != nullptr) \
    { \
        FString SelectedFolder; \
        const FString Title = TEXT("Choose a directory for ImportPath"); \
        const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog( \
            FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr), \
            Title, \
            PathPropertyName, /*default path when open*/ \
            SelectedFolder \
        ); \
        if (bFolderSelected && !SelectedFolder.Equals(PathPropertyName)) \
        { \
            PathPropertyName = SelectedFolder; \
            if (FProperty* ChangedProperty = GetClass()->FindPropertyByName( \
                GET_MEMBER_NAME_CHECKED(UExcelToXmlWidgetBase, PathPropertyName))) \
            { \
                /* This PostEditChange will ensure that ShowFlags is updated. */ \
                FPropertyChangedEvent PropertyChangedEvent(ChangedProperty); \
                this->PostEditChangeProperty(PropertyChangedEvent); \
            } \
        } \
    } \
}

IMPLEMENT_ON_PATH_BUTTON_CLICKED(ImportPath)
IMPLEMENT_ON_PATH_BUTTON_CLICKED(ExportPathForClient)
IMPLEMENT_ON_PATH_BUTTON_CLICKED(ExportPathForServer)

#undef IMPLEMNT_ON_PATH_BUTTON_CLICKED

//void UExcelToXmlWidgetBase::OnImportPathButtonClicked()
//{
//    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
//    if (DesktopPlatform != nullptr)
//    {
//        FString SelectedFolder;
//        const FString Title = TEXT("Choose a directory for ImportPath");
//        const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
//            FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
//            Title,
//            ImportPath, /*default path when open*/
//            SelectedFolder
//        );
//
//        if (bFolderSelected && !SelectedFolder.Equals(ImportPath))
//        {
//            ImportPath = SelectedFolder;
//
//            if (FProperty* ChangedProperty = GetClass()->FindPropertyByName(
//                GET_MEMBER_NAME_CHECKED(UExcelToXmlWidgetBase, ImportPath)))
//            {
//                // This PostEditChange will ensure that ShowFlags is updated.
//                FPropertyChangedEvent PropertyChangedEvent(ChangedProperty);
//                this->PostEditChangeProperty(PropertyChangedEvent);
//            }
//        }
//    }
//}

//void UExcelToXmlWidgetBase::OnExportPathForClientButtonClicked()
//{
//    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
//    if (DesktopPlatform != nullptr)
//    {
//        FString SelectedFolder;
//        const FString Title = TEXT("Choose a directory for ImportPath");
//        const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
//            FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
//            Title,
//            ExportPathForClient,
//            SelectedFolder
//        );
//
//        if (bFolderSelected && !SelectedFolder.Equals(ExportPathForClient))
//        {
//            ExportPathForClient = SelectedFolder;
//
//            if (FProperty* ChangedProperty = GetClass()->FindPropertyByName(
//                GET_MEMBER_NAME_CHECKED(UExcelToXmlWidgetBase, ExportPathForClient)))
//            {
//                // This PostEditChange will ensure that ShowFlags is updated.
//                FPropertyChangedEvent PropertyChangedEvent(ChangedProperty);
//                this->PostEditChangeProperty(PropertyChangedEvent);
//            }
//        }
//    }
//}
//
//void UExcelToXmlWidgetBase::OnExportPathForServerButtonClicked()
//{
//    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
//    if (DesktopPlatform != nullptr)
//    {
//        FString SelectedFolder;
//        const FString Title = TEXT("Choose a directory for ImportPath");
//        const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
//            FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
//            Title,
//            ExprotPathForServer,
//            SelectedFolder
//        );
//
//        if (bFolderSelected && !SelectedFolder.Equals(ExprotPathForServer))
//        {
//            ExprotPathForServer = SelectedFolder;
//
//            if (FProperty* ChangedProperty = GetClass()->FindPropertyByName(
//                GET_MEMBER_NAME_CHECKED(UExcelToXmlWidgetBase, ExprotPathForServer)))
//            {
//                // This PostEditChange will ensure that ShowFlags is updated.
//                FPropertyChangedEvent PropertyChangedEvent(ChangedProperty);
//                this->PostEditChangeProperty(PropertyChangedEvent);
//            }
//        }
//    }
//}

void UExcelToXmlWidgetBase::OnRefreshButtonClicked()
{
    Refresh();
}

void UExcelToXmlWidgetBase::OnXmlExportButtonClicked()
{
    ensure(Subsystem);

    UXlsxData* XlsxData = XlsxListView->GetSelectedItem<UXlsxData>();

    UTableEntryWidgetBase* SelectedWidget = TableListView->GetSelectedItem<UTableEntryWidgetBase>();

    if (XlsxData == nullptr || SelectedWidget == nullptr)
    {
        // TODO 실패 로그
        return;
    }

    /** xlsx 파일 내부에 있는 모든 구조체에 대해서 하나의 xml파일로 출력 */
    //FString PureFilename = FPaths::GetBaseFilename(XlsxData->Filename); // 확장자가 제거된 파일이름
    //FString XmlFilePathForClient = ExportPathForClient / PureFilename + TEXT(".xml");
    //Subsystem->ExportToXmlFile(XmlFilePathForClient, XlsxData);

    /** xlsx 파일 내부에 있는 구조체 중 하나만 xml파일로 출력 */
    FString StructureName = SelectedWidget->GetTextToString();
    FString XmlFilePathForClient = ExportPathForClient / StructureName + TEXT(".xml");
    if (Subsystem->ExportToXmlFile(XmlFilePathForClient, XlsxData, StructureName))
    {
        SetLogText(FString::Printf(TEXT("'%s' 파일이 정상적으로 저장되었습니다."), *XmlFilePathForClient), FColor::Green);
    }
    else
    {
        SetLogText(FString::Printf(TEXT("'%s' 저장에 실패했습니다. 출력로그를 확인하십시오."), *XmlFilePathForClient), FColor::Green);
    }
}

void UExcelToXmlWidgetBase::UpdateExportButton()
{
    ensure(Subsystem);

    UXlsxData* XlsxData = Subsystem->GetWidgetBase()->XlsxListView->GetSelectedItem<UXlsxData>();

    if (XlsxData == nullptr)
    {
        Subsystem->GetWidgetBase()->XmlExportButton->SetIsEnabled(false);
        return;
    }

    // 선택된 xlsx 파일 데이터 시트에 추출할 데이터가 존재하면 'XML Export' 버튼 활성화
    bool bExistToExportData = false;
    if (XlsxData && XlsxData->StructList.Num())
    {
        for (const FXlsxStruct& Struct : XlsxData->StructList)
        {
            for (const TArray<FString>& RowData : Struct.SheetData)
            {
                bExistToExportData = true;
                break;
            }
            if (bExistToExportData)
            {
                break;
            }
        }
    }

    if (bExistToExportData && TableListView->GetSelectedItem<UTableEntryWidgetBase>() != nullptr)
    {
        Subsystem->GetWidgetBase()->XmlExportButton->SetIsEnabled(true);
    }
    else
    {
        Subsystem->GetWidgetBase()->XmlExportButton->SetIsEnabled(false);
    }
}

void UExcelToXmlWidgetBase::Refresh()
{
    XlsxDataList.Empty();

    XlsxListView->SetItemSelection(XlsxListView->GetSelectedItem(), false, ESelectInfo::Direct);
    TableListView->SetItemSelection(TableListView->GetSelectedItem(), false, ESelectInfo::Direct);
	FieldListView->SetItemSelection(FieldListView->GetSelectedItem(), false, ESelectInfo::Direct);

    XlsxListView->ClearListItems();
    TableListView->ClearListItems();
    FieldListView->ClearListItems();

    //UpdateExportButton(); //위에서 XlsxListView->ClearListItems();이 바로 반영이 되지 않아서 UButton::SetIsEnabled(boo)을 직접 호출해준다.
    Subsystem->GetWidgetBase()->XmlExportButton->SetIsEnabled(false);

    ImportPathTextBox->SetText(FText::FromString(ImportPath));
    Subsystem->ParseAndGenerateToXlsxDataList(ImportPath, XlsxDataList);

    for (UXlsxData* Item : XlsxDataList)
    {
        XlsxListView->AddItem(Item);
    }
    SetLogText(TEXT("새로고침 되었습니다."));
}

void UExcelToXmlWidgetBase::SetLogText(const FString& LogStr, FColor TextColor/* = FColor::White*/, ELogVerbosity::Type Verbosity/* = ELogVerbosity::Type::Log*/)
{
    switch (Verbosity)
    {
    case ELogVerbosity::Type::Log:
        UE_LOG(LogExcelToXmlTool, Log, TEXT("%s"), *LogStr);
        break;
    case ELogVerbosity::Type::Verbose:
        UE_LOG(LogExcelToXmlTool, Verbose, TEXT("%s"), *LogStr);
        break;
    case ELogVerbosity::Type::Warning:
        UE_LOG(LogExcelToXmlTool, Warning, TEXT("%s"), *LogStr);
        break;
    case ELogVerbosity::Type::Error:
        UE_LOG(LogExcelToXmlTool, Error, TEXT("%s"), *LogStr);
        break;
    default:
        UE_LOG(LogExcelToXmlTool, All, TEXT("%s"), *LogStr);
		break;
    }
    LogTextBlock->SetColorAndOpacity(FSlateColor(TextColor));
    LogTextBlock->SetText(FText::FromString(LogStr));

    GetWorld()->GetTimerManager().SetTimer(LogVisibilityTimerHandle, FTimerDelegate::CreateLambda([this]()
        {
            LogTextBlock->SetText(FText::GetEmpty());
        }), LogVisibleTime, false);
}

void UExcelToXmlWidgetBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    ensure(Subsystem);

	const FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UExcelToXmlWidgetBase, ImportPath))
	{
        Refresh();
        SetLogText(FString::Printf(TEXT("'%s' 경로로부터 %d 개의 파일을 찾았습니다."), *ImportPath, XlsxDataList.Num()));
	}

    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UExcelToXmlWidgetBase, ExportPathForClient))
    {
        ExportPathForClientTextBox->SetText(FText::FromString(ExportPathForClient));
        SetLogText(FString::Printf(TEXT("'%s' 으로 xml파일의 저장경로가 설정되었습니다."), *ExportPathForClient));
    }

    else if (PropertyName == GET_MEMBER_NAME_CHECKED(UExcelToXmlWidgetBase, ExportPathForServer))
    {
        ExportPathForServerTextBox->SetText(FText::FromString(ExportPathForServer));
    }
    SaveConfig();
}

TSubclassOf<UWidget> UExcelToXmlWidgetBase::XlsxEntryWidgetClass_BP;

TSubclassOf<UWidget> UExcelToXmlWidgetBase::TableEntryWidgetClass_BP;

TSubclassOf<UWidget> UExcelToXmlWidgetBase::FieldEntryWidgetClass_BP;

FString UEntryWidgetBaseCommon::GetTextToString()
{
    return EntryText->GetText().ToString();
}

void UEntryWidgetBaseCommon::InitEntryWidget(const FText& Text)
{
    EntryText->SetText(Text);
    ItemSelectedBlueColorImage->SetVisibility(ESlateVisibility::Collapsed);
}

void UEntryWidgetBaseCommon::NativeOnItemExpansionChanged(bool bIsExpanded)
{
    ItemSelectedBlueColorImage->SetVisibility(ESlateVisibility::Collapsed);
}

void UEntryWidgetBaseCommon::NativeOnEntryReleased()
{
    ItemSelectedBlueColorImage->SetVisibility(ESlateVisibility::Collapsed);
}

void UEntryWidgetBaseCommon::NativeOnItemSelectionChanged(bool bIsSelected)
{
	if (bIsSelected)
	{
		ItemSelectedBlueColorImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		ItemSelectedBlueColorImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UXlsxEntryWidgetBase::NativeOnItemSelectionChanged(bool bIsSelected)
{
    Super::NativeOnItemSelectionChanged(bIsSelected);

    ensure(XlsxDataObj);

    TObjectPtr<class UExcelToXMLSubSystem> Subsystem = GEditor->GetEditorSubsystem<UExcelToXMLSubSystem>();
    ensure(Subsystem);

    if (bIsSelected)
    {
        Subsystem->GetWidgetBase()->TableListView->ClearListItems();
        Subsystem->GetWidgetBase()->FieldListView->ClearListItems();

        Subsystem->GetWidgetBase()->TableListView->SetItemSelection(Subsystem->GetWidgetBase()->TableListView->GetSelectedItem(), false, ESelectInfo::Direct);
        Subsystem->GetWidgetBase()->FieldListView->SetItemSelection(Subsystem->GetWidgetBase()->FieldListView->GetSelectedItem(), false, ESelectInfo::Direct);

        for (const FXlsxStruct& StructData : XlsxDataObj->StructList)
        {
			UTableEntryWidgetBase* NewEntryWidget = Subsystem->GetWidgetBase()->WidgetTree->ConstructWidget<UTableEntryWidgetBase>(UExcelToXmlWidgetBase::TableEntryWidgetClass_BP.Get());

			if (NewEntryWidget)
			{			           
                /**
                * 여기서 적용한 텍스트는 실제로 에디터상에 보이는 위젯에 적용되지 않는다.
                * AddItem을 하면 아이템 리스트에 아이템으로 등록이 되는것이고
                * 위젯은 ListView에 등록된 EntryClass 타입의 CDO로 부터 객체가 생성되는 것으로 보인다.
                * AddItem을 하면 ListView내에 아이템과 위젯으로 개별적인 오브젝트로 각각 등록이 되어 있다고 보면된다.
                * 따라서 이상해 보일 수 있지만 UTableEntryWidgetBase::NativeOnListItemObjectSet 에서
                * 아이템으로 들어오는 객체의 멤버를 위젯으로 넣어주는 작업을 해줘야한다.
                */
                NewEntryWidget->EntryText->SetText(FText::FromString(StructData.Name));  
                Subsystem->GetWidgetBase()->TableListView->AddItem(NewEntryWidget);
            }
        }

        Subsystem->GetWidgetBase()->UpdateExportButton();
    }
}

void UXlsxEntryWidgetBase::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (XlsxDataObj = Cast<UXlsxData>(ListItemObject))
    {
        InitEntryWidget(FText::FromString(XlsxDataObj->Filename));
    }
}

void UTableEntryWidgetBase::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UTableEntryWidgetBase* ItemWidget = Cast<UTableEntryWidgetBase>(ListItemObject))
    {
        InitEntryWidget(ItemWidget->EntryText->GetText());
    }
}

void UFieldEntryWidgetBase::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    if (UFieldEntryWidgetBase* ItemWidget = Cast<UFieldEntryWidgetBase>(ListItemObject))
    {
        InitEntryWidget(ItemWidget->EntryText->GetText());
    }
}

void UTableEntryWidgetBase::NativeOnItemSelectionChanged(bool bIsSelected)
{
    Super::NativeOnItemSelectionChanged(bIsSelected);

    TObjectPtr<class UExcelToXMLSubSystem> Subsystem = GEditor->GetEditorSubsystem<UExcelToXMLSubSystem>();
    ensure(Subsystem);

    UXlsxData* XlsxData = Subsystem->GetWidgetBase()->XlsxListView->GetSelectedItem<UXlsxData>();

    if (bIsSelected && XlsxData)
    {
        Subsystem->GetWidgetBase()->FieldListView->ClearListItems();

        FString WidgetText = EntryText->GetText().ToString();
        
        if (FXlsxStruct* XlsxStruct = XlsxData->StructList.FindByKey(WidgetText))
        {
            for (const TPair<FString, FString>& FieldPair : XlsxStruct->FieldList)
            {
				UFieldEntryWidgetBase* NewEntryWidget = Subsystem->GetWidgetBase()->WidgetTree->ConstructWidget<UFieldEntryWidgetBase>(UExcelToXmlWidgetBase::FieldEntryWidgetClass_BP.Get());

				if (NewEntryWidget)
				{
                    /**
                    * UXlsxEntryWidgetBase::NativeOnItemSelectionChanged(bool bIsSelected) 함수내용안에 있는 주석과 마찬가지의 작업필요. Line 276 주석 참고.
                    */
                    NewEntryWidget->EntryText->SetText(FText::FromString(FieldPair.Key));
                    Subsystem->GetWidgetBase()->FieldListView->AddItem(NewEntryWidget);
                }
            }
        }
        Subsystem->GetWidgetBase()->UpdateExportButton();
    }	
}

#undef LOCTEXT_NAMESPACE

