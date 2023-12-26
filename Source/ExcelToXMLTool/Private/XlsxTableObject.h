// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XlsxTableObject.generated.h"

UENUM()
enum class EParsingFileType : uint8
{
	SimpleTable,
	ComplexTable
};

USTRUCT(BlueprintType)
struct FXlsxStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ExcelToXml")
	FString Name;

	/**
	* 키 값은 필드이름, 벨류 값은 필드의 자료형
	* TMap은 이터레이션시 add된 순서대로 반환하는 것이 보장되지 않기 때문에 TArray를 사용
	*/
	TArray<TPair<FString, FString>> FieldList;

	/** FieldList에 들어있는 필드순서에 대응하는 데이터값이 들어있는 Row 배열*/
	TArray<TArray<FString>> SheetData;

	bool operator==(const FString& Rhs) const
	{
		return Name.Equals(Rhs, ESearchCase::CaseSensitive);
	}
};

USTRUCT(BlueprintType)
struct FXlsxEnum
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ExcelToXml")
	FString Name;

	/** 키 값은 필드이름, 벨류 값은 Enum타입의 값(정수)  */
	TArray<TPair<FString, uint32>> FieldList;

	bool operator==(const FString& Rhs) const
	{
		return Name.Equals(Rhs, ESearchCase::IgnoreCase);
	}
};

USTRUCT()
struct FSimpleTable
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "ExcelToXml")
	TArray<FString> FieldList;

	//UPROPERTY()
	TArray<TArray<FString>> DataSheet;
};


/**
 * 
 */
UCLASS(BlueprintType)
class UXlsxData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "ExcelToXml")
	EParsingFileType Type;

	UPROPERTY(BlueprintReadOnly, Category = "ExcelToXml")
	FString Filepath;

	UPROPERTY(BlueprintReadOnly, Category = "ExcelToXml")
	FString Filename;

	UPROPERTY(BlueprintReadOnly, Category = "ExcelToXml")
	TArray<FXlsxStruct> StructList;

	UPROPERTY(BlueprintReadOnly, Category = "ExcelToXml")
	TArray<FXlsxEnum> EnumList;

public:
};