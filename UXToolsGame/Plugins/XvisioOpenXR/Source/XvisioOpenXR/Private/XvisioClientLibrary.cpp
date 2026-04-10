
/*
 * @author: eddy
 * @Date: 2025-03-18 11:45:47
 * @LastEditTime: 2025-04-22 15:41:32
 * @LastEditors: eddy
 * @brief: 
 * @copyright: Xvisio Tec
 * @FilePath: \ALVR-masteri:\xvisio\ue4mrtk\Source\mrtktest\XvisioClientLibrary.cpp
 * 唵嘛呢叭咪吽
 */

#include "XvisioClientLibrary.h"
#include "Engine/Engine.h"
#include "Json.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
//#include "IXRTrackingSystem.h"
//#include "HeadMountedDisplayFunctionLibrary.h"


void* UXvisioClientLibrary::XvisioDLL = nullptr;
static int mClientId;
static bool isDllLoaded = false;
UXvisioClientLibrary::CreateClientAPI UXvisioClientLibrary::CreateClientFunc = nullptr;
UXvisioClientLibrary::CreateClientWithIpAPI UXvisioClientLibrary::CreateClientWithIpFunc = nullptr;
UXvisioClientLibrary::SendMessageAPI UXvisioClientLibrary::SendMessageFunc = nullptr;
UXvisioClientLibrary::ScreenShotAPI UXvisioClientLibrary::ScreenShotFunc = nullptr;
UXvisioClientLibrary::StartRecordAPI UXvisioClientLibrary::StartRecordFunc = nullptr;
UXvisioClientLibrary::StopRecordAPI UXvisioClientLibrary::StopRecordFunc = nullptr;
UXvisioClientLibrary::StartCSLAMMapAPI UXvisioClientLibrary::StartCSLAMMapFunc = nullptr;
UXvisioClientLibrary::SetMapPathAPI UXvisioClientLibrary::SetMapPathFunc = nullptr;
UXvisioClientLibrary::SaveSlamMapAPI UXvisioClientLibrary::SaveSlamMapFunc = nullptr;
UXvisioClientLibrary::SendMapFileAPI UXvisioClientLibrary::SendMapFileFunc = nullptr;
UXvisioClientLibrary::StartASRAPI UXvisioClientLibrary::StartASRFunc = nullptr;
UXvisioClientLibrary::StartEyeTrackingAPI UXvisioClientLibrary::StartEyeTrackingFunc = nullptr;
UXvisioClientLibrary::GetEyeReceivedDataAPI UXvisioClientLibrary::GetEyeReceivedDataFunc = nullptr;
UXvisioClientLibrary::GetReceivedMessageAPI UXvisioClientLibrary::GetReceivedMessageFunc = nullptr;
UXvisioClientLibrary::CloseClientAPI UXvisioClientLibrary::CloseClientFunc = nullptr;

EXvisioClientPlatform UXvisioClientLibrary::GetXvisioClientPlatform()
{
#if PLATFORM_WINDOWS

	return EXvisioClientPlatform::Windows;
#elif PLATFORM_ANDROID_ARM
	return EXvisioClientPlatform::Android;
#elif PLATFORM_LINUX
	return EXvisioClientPlatform::Linux;
#else
	return EXvisioClientPlatform::Unknown;
#endif
}

void UXvisioClientLibrary::LoadXvisioDLL()
{
    FString DllPath;

#if PLATFORM_WINDOWS
    DllPath = FPaths::ProjectDir() / TEXT("Binaries/Win64/xv_client.dll");
#elif PLATFORM_LINUX
    DllPath = FPaths::ProjectDir() / TEXT("Binaries/Linux/libxv_client.so");
#else
    // 可以根据需要添加其他平台的处理
    UE_LOG(LogTemp, Warning, TEXT("Unsupported platform"));
#endif
    XvisioDLL = FPlatformProcess::GetDllHandle(*DllPath);

    if (XvisioDLL)
    {
        UE_LOG(LogTemp, Log, TEXT("Loaded xv_client.dll"));

        CreateClientFunc = (CreateClientAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("CreateClientAPI"));
        CreateClientWithIpFunc = (CreateClientWithIpAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("CreateClientWithIpAPI"));
        SendMessageFunc = (SendMessageAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("SendMessageAPI"));
        StartRecordFunc = (StartRecordAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("startRecord"));
        ScreenShotFunc = (ScreenShotAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("screenShort"));
        StopRecordFunc = (StopRecordAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("stopRecord"));
        StartCSLAMMapFunc = (StartCSLAMMapAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("startCslamMap"));
        SetMapPathFunc = (SetMapPathAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("setMapPath"));
        SaveSlamMapFunc = (SaveSlamMapAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("SaveSlamMap"));
        SendMapFileFunc = (SendMapFileAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("sendMapFile"));
        StartASRFunc = (StartASRAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("startASR"));
        StartEyeTrackingFunc = (StartEyeTrackingAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("startEyeTracking"));
        GetEyeReceivedDataFunc = (GetEyeReceivedDataAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("getEyeReceivedData"));
        GetReceivedMessageFunc = (GetReceivedMessageAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("GetReceivedMessageAPI"));
        CloseClientFunc = (CloseClientAPI)FPlatformProcess::GetDllExport(XvisioDLL, TEXT("CloseClient"));

        int clientId = CreateClient();
      //  FPlatformProcess::Sleep(6.0f);
        
  
        mClientId = clientId;
        UE_LOG(LogTemp, Log, TEXT("xv_client mClientId: %d"), mClientId);

        //xvisio test start 
      //  FString LOCALBNF = TEXT("#BNF+IAT 1.0 UTF-8;\n!grammar word;\n!slot <words>;\n!start <words>;\n<words>:打开!id(999)|关闭!id(1000)|返回!id(1001)|前进!id(1002)|后退!id(1003)|放大!id(1004);\n"); 
        FString LOCALBNF = TEXT("#BNF+IAT 1.0 UTF-8;\n!grammar word;\n!slot <words>;\n!start <words>;\n<words>:放大!id(999)|缩小!id(1000)|左转!id(1001)|右转!id(1002)|后退!id(1003);\n");
        StartASR(LOCALBNF);
   //     StartCSLAMMap();
  //      FPlatformProcess::Sleep(6.0f);
   //     SaveSlamMap("C:\\Users\\zhou\\Downloads\\","test123.bin");
        ScreenShot();
    //    StartRecord();
    //    StartCSLAMMap();
     //   SaveSlamMap(clientId, "");
    //    SendMapFile(clientId, "");
        //xvisio test end
        isDllLoaded = true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load xv_client.dll"));
    }
}

void UXvisioClientLibrary::FreeXvisioDLL()
{
    if (XvisioDLL)
    {
        FPlatformProcess::FreeDllHandle(XvisioDLL);
        XvisioDLL = nullptr;
    }
}

// 完整封装
int UXvisioClientLibrary::CreateClient()
{
    if (CreateClientFunc)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateClientFunc is not null"));
        return CreateClientFunc();
    }
    return -1;
}
int UXvisioClientLibrary::CreateClientWithIp(const FString& ip)
{
    if (CreateClientWithIpFunc)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateClientFunc is not null"));
        return CreateClientWithIpFunc(TCHAR_TO_ANSI(*ip));
    }
    return -1;
}
void UXvisioClientLibrary::SendMessage( const FString& Action, const FString& Message)
{
    if (SendMessageFunc)
    {
        SendMessageFunc(mClientId, TCHAR_TO_ANSI(*Action), TCHAR_TO_ANSI(*Message));
    }
}
void UXvisioClientLibrary::ScreenShot()
{
    UE_LOG(LogTemp, Log, TEXT("ScreenShot entry 1"));
    if (ScreenShotFunc)
    {
        UE_LOG(LogTemp, Log, TEXT("ScreenShot entry 2"));
        ScreenShotFunc(mClientId);
    }
}
void UXvisioClientLibrary::StartRecord()
{
    UE_LOG(LogTemp, Log, TEXT("StartRecord entry"));
    if (StartRecordFunc)
    {
        UE_LOG(LogTemp, Log, TEXT("StartRecord entry"));
        StartRecordFunc(mClientId);
        UE_LOG(LogTemp, Log, TEXT("StartRecord done"));
    }
}

void UXvisioClientLibrary::StopRecord()
{
    if (StopRecordFunc)
    {
        StopRecordFunc(mClientId);
    }
}

void UXvisioClientLibrary::StartCSLAMMap()
{
    if (StartCSLAMMapFunc)
    {
        UE_LOG(LogTemp, Error, TEXT("StartCSLAMMapFunc entry"));
        StartCSLAMMapFunc(mClientId);
    }
}

void UXvisioClientLibrary::SaveSlamMap(const FString& pcPath,const FString& fileName)
{
    UE_LOG(LogTemp, Error, TEXT("SaveSlamMap start"));
    if (SaveSlamMapFunc)
    {
   //     FString saveName ="testmap0319.bin";
   //     UE_LOG(LogTemp, Error, TEXT("SaveSlamMap entry"));
        SaveSlamMapFunc(mClientId, TCHAR_TO_ANSI(*pcPath),TCHAR_TO_ANSI(*fileName));
    }
}

void UXvisioClientLibrary::SendMapFile( const FString& FilePath)
{
    if (SendMapFileFunc)
    {
        UE_LOG(LogTemp, Log, TEXT("SendMapFileFunc entry"));
       // FString filePath = TEXT("C:\\Users\\Public\\Downloads\\testmap0319.bin");
        SendMapFileFunc(mClientId, TCHAR_TO_ANSI(*FilePath));
        UE_LOG(LogTemp, Log, TEXT("SendMapFileFunc done"));
    }
}

void UXvisioClientLibrary::StartASR( const FString& bnf)
{
    if (StartASRFunc)
    {
        FTCHARToUTF8 UTF8String(*bnf);
        UE_LOG(LogTemp, Log, TEXT("UTF-8 String: %s"), *bnf);
        StartASRFunc(mClientId, TCHAR_TO_UTF8(*bnf));
    }
}



void UXvisioClientLibrary::StartEyeTracking()
{
    if (StartEyeTrackingFunc)
    {
        UE_LOG(LogTemp, Error, TEXT("StartEyeTrackingFunc entry"));
        StartEyeTrackingFunc(mClientId);
    }
}

//void UXvisioClientLibrary::SetPcSaveMapPath(const FString& filePath)
//{
//    if (SetMapPathFunc)
//    {
//        SetMapPathFunc(mClientId, TCHAR_TO_ANSI(*filePath));
//    }
//}

// 传入 JSON 字符串，解析并打印向量数据
void ParseAndPrintEyeData(const FString& JsonString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        const TArray<TSharedPtr<FJsonValue>>* PositionArrayPtr = nullptr;
        const TArray<TSharedPtr<FJsonValue>>* OrientationArrayPtr = nullptr;

        bool bHasPosition = JsonObject->TryGetArrayField(TEXT("position"), PositionArrayPtr);
        bool bHasOrientation = JsonObject->TryGetArrayField(TEXT("orientation"), OrientationArrayPtr);

        if (bHasPosition && bHasOrientation && PositionArrayPtr && OrientationArrayPtr &&
            PositionArrayPtr->Num() >= 3 && OrientationArrayPtr->Num() >= 4)
        {
            FVector Position(
                (float)(*PositionArrayPtr)[0]->AsNumber(),
                (float)(*PositionArrayPtr)[1]->AsNumber(),
                (float)(*PositionArrayPtr)[2]->AsNumber()
            );

            FQuat Orientation(
                (float)(*OrientationArrayPtr)[0]->AsNumber(),
                (float)(*OrientationArrayPtr)[1]->AsNumber(),
                (float)(*OrientationArrayPtr)[2]->AsNumber(),
                (float)(*OrientationArrayPtr)[3]->AsNumber()
            );

            UE_LOG(LogTemp, Log, TEXT("EyeData Position: %s"), *Position.ToString());
            UE_LOG(LogTemp, Log, TEXT("EyeData Orientation: %s"), *Orientation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Missing or invalid 'position' or 'orientation' field."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON."));
    }

}
bool UXvisioClientLibrary::GetEyeReceivedData(FEyeData& OutEyeData)
{
    FQuat Orientation;
    FVector Position;
    //if (GEngine->XRSystem.IsValid())
    //{
    //    GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, Orientation, Position);
    //    // 打印位置
    //    UE_LOG(LogTemp, Log, TEXT("HMD Position: X=%f, Y=%f, Z=%f"), Position.X, Position.Y, Position.Z);

    //    // 打印旋转四元数
    //    UE_LOG(LogTemp, Log, TEXT("HMD Orientation (Quat): X=%f, Y=%f, Z=%f, W=%f"),
    //        Orientation.X, Orientation.Y, Orientation.Z, Orientation.W);

    //    // 也可以转换为欧拉角打印
    //    FRotator Rot = Orientation.Rotator();
    //    UE_LOG(LogTemp, Log, TEXT("HMD Orientation (Rotator): Pitch=%f, Yaw=%f, Roll=%f"),
    //        Rot.Pitch, Rot.Yaw, Rot.Roll);
    //}
    if (!GetEyeReceivedDataFunc)
    {
        return false;
    }
    if (!isDllLoaded) {
        UE_LOG(LogTemp, Error, TEXT("isDllLoaded is false"));
        return false;
    }
    const char* result = GetEyeReceivedDataFunc();
    if (!result || strlen(result) < 10) return false;

    FString ResultString = FString(UTF8_TO_TCHAR(result));
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResultString);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("JSON parse failed: %s"), *ResultString);
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* PositionArray = nullptr;
    const TArray<TSharedPtr<FJsonValue>>* OrientationArray = nullptr;

    if (!JsonObject->TryGetArrayField(TEXT("position"), PositionArray) ||
        !JsonObject->TryGetArrayField(TEXT("orientation"), OrientationArray))
    {
        UE_LOG(LogTemp, Error, TEXT("Missing JSON fields"));
        return false;
    }

    if (!PositionArray || PositionArray->Num() < 3 || !OrientationArray || OrientationArray->Num() < 4)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid array size"));
        return false;
    }

    OutEyeData.Position = FVector(
        (float)(*PositionArray)[0]->AsNumber(),
        (float)(*PositionArray)[1]->AsNumber(),
        (float)(*PositionArray)[2]->AsNumber() + 150
    );

    OutEyeData.PositionEnd = FVector(
        (float)(*OrientationArray)[0]->AsNumber(),
        (float)(*OrientationArray)[1]->AsNumber(),
        (float)(*OrientationArray)[2]->AsNumber() + 150
    );
    //float Distance = 300.0f;

    //// 获取“前方”方向向量（Z轴为前）
    //FVector Forward = OutEyeData.Orientation.GetForwardVector(); // 等价于 Unity 的 transform.forward

    //// 计算目标点
    //FVector TargetPos = OutEyeData.Position + Forward * Distance;
    //OutEyeData.Orientation = FQuat(
    //    TargetPos.X,
    //    TargetPos.Y,
    //    TargetPos.Z,
    //    1
    //);
    // 打印转换后的数据
    UE_LOG(LogTemp, Log, TEXT("Eye Position: X=%.3f, Y=%.3f, Z=%.3f"),
        OutEyeData.Position.X, OutEyeData.Position.Y, OutEyeData.Position.Z);
    UE_LOG(LogTemp, Log, TEXT("Eye Orientation: X=%.3f, Y=%.3f, Z=%.3f"),
        OutEyeData.PositionEnd.X, OutEyeData.PositionEnd.Y,
        OutEyeData.PositionEnd.Z);
    return true;
}





void LogUnicodeMessage(const FString& Message)
{
    GLog->Serialize(*Message, ELogVerbosity::Log, FName(TEXT("LogTemp")));
}

// 解析 msg JSON 并提取识别的单词
void ParseAsrResultMessage(const FString& MsgJsonString)
{
    // 解析 msg JSON
    TSharedPtr<FJsonObject> MsgJsonObject;
    TSharedRef<TJsonReader<>> MsgReader = TJsonReaderFactory<>::Create(MsgJsonString);

    if (!FJsonSerializer::Deserialize(MsgReader, MsgJsonObject) || !MsgJsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse 'msg' as JSON: %s"), *MsgJsonString);
        return;
    }

    // 获取 ws 数组
    const TArray<TSharedPtr<FJsonValue>>* WsArray;
    if (MsgJsonObject->TryGetArrayField(TEXT("ws"), WsArray))
    {
        for (const TSharedPtr<FJsonValue>& WsElement : *WsArray)
        {
            const TSharedPtr<FJsonObject>* WsObject;
            if (WsElement->TryGetObject(WsObject) && WsObject->IsValid())
            {
                // 获取 cw 数组
                const TArray<TSharedPtr<FJsonValue>>* CwArray;
                if ((*WsObject)->TryGetArrayField(TEXT("cw"), CwArray))
                {
                    for (const TSharedPtr<FJsonValue>& CwElement : *CwArray)
                    {
                        const TSharedPtr<FJsonObject>* CwObject;
                        if (CwElement->TryGetObject(CwObject) && CwObject->IsValid())
                        {
                            FString id;
                            if ((*CwObject)->TryGetStringField(TEXT("id"), id))
                            {
                                LogUnicodeMessage(id);
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("'ws' field missing in msg JSON"));
    }
}
FString UXvisioClientLibrary::GetReceivedMessage()
{
    if (GetReceivedMessageFunc)
    {
        const char* result = GetReceivedMessageFunc(mClientId);
        FString Message(result);

        // 解析 JSON
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            FString ActionValue;
            if (JsonObject->TryGetStringField(TEXT("action"), ActionValue))
            {
                // 判断 action 值，并打印对应日志
                if (ActionValue.Equals(TEXT("loadMapDone")))
                {
                    UE_LOG(LogTemp, Log, TEXT("Action is loadMapDone"));
                }
                else if (ActionValue.Equals(TEXT("saveMapDone")))
                {
                    UE_LOG(LogTemp, Log, TEXT("Action is saveMapDone"));
                }
                else if(ActionValue.Equals(TEXT("onAsrResult")))
                {
                    FString RawMsgValue;
                    if (JsonObject->TryGetStringField(TEXT("msg"), RawMsgValue))
                    {
                        // 处理 msg 可能存在的 "|true" 额外数据
                        int32 PipeIndex;
                        if (RawMsgValue.FindChar('|', PipeIndex))
                        {
                            RawMsgValue = RawMsgValue.Left(PipeIndex);
                        }

                        // 解析 msg 字符串为 JSON
                        FString DecodedMsgValue = RawMsgValue.Replace(TEXT("\\n"), TEXT("")).Replace(TEXT("\\\""), TEXT("\""));

                        UE_LOG(LogTemp, Log, TEXT("onAsrResult msg (Raw): %s"), *RawMsgValue);
                        UE_LOG(LogTemp, Log, TEXT("onAsrResult msg (Decoded): %s"), *DecodedMsgValue);

                        ParseAsrResultMessage(DecodedMsgValue);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("'msg' field missing in onAsrResult JSON"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("Action is unknown: %s"), *ActionValue);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to get 'action' field"));
            }
        }
        else
        {
          //  UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
        }

        return Message;
    }
    return FString();
}

void UXvisioClientLibrary::CloseClient()
{
    if (CloseClientFunc)
    {
        CloseClientFunc(mClientId);
    }
}
