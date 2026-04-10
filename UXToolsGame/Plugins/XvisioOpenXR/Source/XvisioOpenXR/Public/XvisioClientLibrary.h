
/*
 * @author: eddy
 * @Date: 2025-03-18 11:45:47
 * @LastEditTime: 2025-03-18 11:46:08
 * @LastEditors: eddy
 * @brief: 
 * @copyright: Xvisio Tec
 * @FilePath: \mydlle:\Unreal Projects\mrtktest1101\ue4mrtk\Source\mrtktest\XvisioClientLibrary.h
 * 唵嘛呢叭咪吽
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XvisioClientLibrary.generated.h"
USTRUCT(BlueprintType)
struct XVISIOOPENXR_API FEyeData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly)
        FVector Position;

    UPROPERTY(BlueprintReadOnly)
        FVector PositionEnd;
};

UENUM(BlueprintType)
enum class EXvisioClientPlatform:uint8
{
	Unknown,
	Linux,
	Windows,
	Android
};

UCLASS()
class XVISIOOPENXR_API UXvisioClientLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Xvisio")
	static EXvisioClientPlatform GetXvisioClientPlatform();
    /**
	* 加载DLL，只在桌面端有效
	*/
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void LoadXvisioDLL();
	/**
	 * 释放DLL，只在桌面端有效
	 */
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void FreeXvisioDLL();

    // 客户端相关API
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static int CreateClient();
    //ip为box端的ip地址
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static int CreateClientWithIp(const FString& ip);
    //自定义发送action命令
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void SendMessage( const FString& Action, const FString& Message);
    //截屏命令
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void ScreenShot();
    //开始录屏
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void StartRecord();
    //结束录屏
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void StopRecord();
    //开启cslam地图
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void StartCSLAMMap();
    //保存cslam地图 pcPath:电脑的地图文件路径  fileName:地图名称
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void SaveSlamMap(const FString& pcPath, const FString& fileName);
    //设置保存在电脑的地图文件路径
  /*  UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void SetPcSaveMapPath(const FString& FilePath);*/
    //发送电脑cslam地图文件到盒子端
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void SendMapFile( const FString& FilePath);
    //开启语音识别 bnf为自定义语义id
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void StartASR( const FString& bnf);
    //开启眼控
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void StartEyeTracking();
    //获取眼控数据
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
     static bool GetEyeReceivedData(FEyeData& OutEyeData);
    //获取语音识别结果
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static FString GetReceivedMessage();
    //关闭xv client
    UFUNCTION(BlueprintCallable, Category = "Xvisio")
    static void CloseClient();

private:
    // DLL 句柄
    static void* XvisioDLL;

    // 定义API函数指针类型
    typedef int(*CreateClientAPI)();
    typedef int(*CreateClientWithIpAPI)(const char* ip);
    typedef void(*SendMessageAPI)(int ClientId, const char* action, const char* message);
    typedef void(*ScreenShotAPI)(int ClientId);
    typedef void(*StartRecordAPI)(int ClientId);
    typedef void(*StopRecordAPI)(int ClientId);
    typedef void(*StartCSLAMMapAPI)(int ClientId);
    typedef void (*SetMapPathAPI)(int ClientId, const char* filePath);
    typedef void(*SaveSlamMapAPI)(int ClientId, const char* pcPath, const char* fileName);
    typedef void(*SendMapFileAPI)(int ClientId, const char* filePath);
    typedef void(*StartASRAPI)(int ClientId, const char* bnf);
    typedef void(*StartEyeTrackingAPI)(int ClientId);
    typedef const char*(*GetEyeReceivedDataAPI)();
    typedef const char*(*GetReceivedMessageAPI)(int ClientId);
    typedef void(*CloseClientAPI)(int ClientId);

    // 定义函数指针
    static CreateClientAPI CreateClientFunc;
    static CreateClientWithIpAPI CreateClientWithIpFunc;
    static SendMessageAPI SendMessageFunc;
    static ScreenShotAPI ScreenShotFunc;
    static StartRecordAPI StartRecordFunc;
    static StopRecordAPI StopRecordFunc;
    static StartCSLAMMapAPI StartCSLAMMapFunc;
    static SaveSlamMapAPI SaveSlamMapFunc;
    static SetMapPathAPI SetMapPathFunc;
    static SendMapFileAPI SendMapFileFunc;
    static StartASRAPI StartASRFunc;
    static StartEyeTrackingAPI StartEyeTrackingFunc;
    static GetEyeReceivedDataAPI GetEyeReceivedDataFunc;
    static GetReceivedMessageAPI GetReceivedMessageFunc;
    static CloseClientAPI CloseClientFunc;
};
