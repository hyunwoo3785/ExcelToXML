// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Runtime/UMG/Public/Blueprint/IUserObjectListEntry.h"
#include <Logging/LogVerbosity.h>
#include "ExcelToXmlWidgetBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogExcelToXmlTool, Log, All)
/**
 * 
 */
UCLASS(Abstract, Config=EditorPerProjectUserSettings)
class UExcelToXmlWidgetBase : public UEditorUtilityWidget
{
	GENERATED_UCLASS_BODY()

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;

private:
	/** 
	* 다이나믹 델리게이트에 등록되는 함수들은 UFUNCTION()을 붙여줘야한다.
	*/
	UFUNCTION()
	void OnImportPathButtonClicked();
	UFUNCTION()
	void OnExportPathForClientButtonClicked();
	UFUNCTION()
	void OnExportPathForServerButtonClicked();
	UFUNCTION()
	void OnRefreshButtonClicked();
	UFUNCTION()
	void OnXmlExportButtonClicked();

	void UpdateExportButton();

	void Refresh();

	void SetLogText(const FString& Log, FColor TextColor = FColor::White, ELogVerbosity::Type Verbosity = ELogVerbosity::Type::Log);
public:
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
	/** EditableTextBox */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableTextBox> ImportPathTextBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableTextBox> ExportPathForClientTextBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableTextBox> ExportPathForServerTextBox;

	/** Path button for import & export */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ImportPathButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ExportPathForClientButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ExportPathForServerButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> RefreshButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> XmlExportButton;

	/** List view */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UListView> XlsxListView;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UListView> TableListView;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UListView> FieldListView;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> LogTextBlock;

	UPROPERTY(Config)
	FString ImportPath;
	UPROPERTY(Config)
	FString ExportPathForClient;
	UPROPERTY(Config)
	FString ExportPathForServer;

	static TSubclassOf<UWidget> XlsxEntryWidgetClass_BP;
	static TSubclassOf<UWidget> TableEntryWidgetClass_BP;
	static TSubclassOf<UWidget> FieldEntryWidgetClass_BP;

	UPROPERTY()
	TArray<TObjectPtr<class UXlsxData>> XlsxDataList;
	TObjectPtr<class UExcelToXMLSubSystem> Subsystem;

	FTimerHandle	LogVisibilityTimerHandle;
	const float		LogVisibleTime = 5.f; // Second

	friend class UXlsxEntryWidgetBase;
	friend class UTableEntryWidgetBase;
	friend class UFieldEntryWidgetBase;
};

UCLASS(Abstract)
class UEntryWidgetBaseCommon : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	FString GetTextToString();
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemSelectedBlueColorImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> EntryText;

protected:
	void InitEntryWidget(const FText& Text);
	void NativeOnItemExpansionChanged(bool bIsExpanded) override;
	void NativeOnEntryReleased() override;

	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

};

UCLASS(Abstract)
class UXlsxEntryWidgetBase : public UEntryWidgetBaseCommon
{
	GENERATED_BODY()
protected:
	void NativeOnItemSelectionChanged(bool bIsSelected) override;

	void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:
	TObjectPtr<class UXlsxData> XlsxDataObj;
};

UCLASS(Abstract)
class UTableEntryWidgetBase : public UEntryWidgetBaseCommon
{
	GENERATED_BODY()
protected:
	void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void NativeOnItemSelectionChanged(bool bIsSelected) override;

	friend class UXlsxEntryWidgetBase;
};

UCLASS(Abstract)
class UFieldEntryWidgetBase : public UEntryWidgetBaseCommon
{
	GENERATED_BODY()
protected:
	friend class UTableEntryWidgetBase;

	void NativeOnListItemObjectSet(UObject* ListItemObject) override;

};