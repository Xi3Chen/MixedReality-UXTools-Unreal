// Copyright Epic Games, Inc. All Rights Reserved.

#include "XvisioOpenXR.h"
#include "OpenXRCore.h"
#include "External/openxr/openxr.h"
#include "External/openxr/openxr_xvxr.h"
// #include "OpenXRPlatformRHI.h"
#include "DefaultSpectatorScreenController.h"
#include "Modules/ModuleManager.h"

#if PLATFORM_ANDROID
#include "Android/AndroidPlatformMisc.h"
#include <android_native_app_glue.h>
// #include "JNI/RuntimeCheckerWrapper.h"
// #include "Misc/MessageDialog.h"
#include <dlfcn.h>
#endif // PLATFORM_ANDROID
#include "External/openxr/openxr_platform.h"

DECLARE_LOG_CATEGORY_EXTERN(LogXvisioOpenXRPlugin, Log, All);

DEFINE_LOG_CATEGORY(LogXvisioOpenXRPlugin);
static class FXvisioOpenXRHMD *g_XvisioOpenXRModule;

class FXvisioOpenXRHMD : public IXvisioOpenXRPlugin
{
private:
	void *LoaderHandle = nullptr;
	void *ApiHandle = nullptr;

public:
	// FOpenXRHandTracking handTrackingModule;
	FXvisioOpenXRHMD()
		: LoaderHandle(nullptr)
	{
	}

	virtual ~FXvisioOpenXRHMD()
	{
	}
	virtual void StartupModule() override;

	virtual void ShutdownModule() override
	{
		if (LoaderHandle)
		{
			//	handTrackingModule.Unregister();
			FPlatformProcess::FreeDllHandle(LoaderHandle);
			LoaderHandle = nullptr;
		}
		if (ApiHandle)
		{
			//	handTrackingModule.Unregister();
			FPlatformProcess::FreeDllHandle(ApiHandle);
			ApiHandle = nullptr;
		}
	}

	virtual bool GetCustomLoader(PFN_xrGetInstanceProcAddr *OutGetProcAddr) override;
	virtual bool IsStandaloneStereoOnlyDevice() override;
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR *> &OutExtensions) override;
	virtual bool GetInteractionProfile(XrInstance InInstance, FString &OutKeyPrefix, XrPath &OutPath, bool &OutHasHaptics) override;
	virtual bool GetSpectatorScreenController(FHeadMountedDisplayBase *InHMDBase, TUniquePtr<FDefaultSpectatorScreenController> &OutSpectatorScreenController) override;
	// virtual void AddActions(XrInstance Instance, TFunction<XrAction(XrActionType InActionType, const FName& InName, const TArray<XrPath>& InSubactionPaths)> AddAction) override;
	// virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader) override;
	// virtual const void* OnCreateInstance(class IOpenXRHMDPlugin* InPlugin, const void* InNext) override;
	// virtual const void* OnGetSystem(XrInstance InInstance, const void* InNext) override;
	virtual const void *OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void *InNext) override;
	// virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	// virtual const void* OnBeginFrame(XrSession InSession, XrTime DisplayTime, const void* InNext) override;
	// virtual const void* OnBeginProjectionView(XrSession InSession, int32 InLayerIndex, int32 InViewIndex, const void* InNext) override;
	// virtual const void* OnBeginDepthInfo(XrSession InSession, int32 InLayerIndex, int32 InViewIndex, const void* InNext) override;
	virtual const void *OnEndProjectionLayer(XrSession InSession, int32 InLayerIndex, const void *InNext, XrCompositionLayerFlags &OutFlags) override;
	// virtual const void* OnEndFrame(XrSession InSession, XrTime DisplayTime, const TArray<XrSwapchainSubImage> InColorImages, const TArray<XrSwapchainSubImage> InDepthImages, const void* InNext) override;
	// virtual const void* OnSyncActions(XrSession InSession, const void* InNext) override;
	// virtual void PostSyncActions(XrSession InSession) override;
	virtual FString GetDisplayName() override;
	virtual bool IsActive() const;
#if PLATFORM_ANDROID
	virtual void GetRequiredPermissions(TArray<FString> &permissions) const {};
	virtual bool IsRuntimeCameraPermissionsRequired() const { return false; };
#endif

	void SetSessionHandle(const XrSession &inSession);
	void SetInstanceHandle(const XrInstance &inInstance);
	// virtual bool OnSpacesFeatureEnabled();
	// virtual bool OnSpacesFeatureDisabled();
	virtual bool GetRequiredLayers(TSet<const ANSICHAR *> &outLayers)
	{
		return true;
	}

private:
	// static ESpacesExtensionLoadingState IsValidExtension(FSpacesExtension *extension);

	bool _bIsFeatureToggled = false;
	XrSession _SessionHandle = XR_NULL_HANDLE;
	XrInstance _InstanceHandle = XR_NULL_HANDLE;
};

/*bool UXvisioOpenXRFunctionLibrary::SetUseHandMesh(EHandMeshStatus Mode)
{
return false; // g_XvisioOpenXRModule->handTrackingModule.Turn(Mode);
} */
bool FXvisioOpenXRHMD::GetCustomLoader(PFN_xrGetInstanceProcAddr *OutGetProcAddr)
{

#if PLATFORM_ANDROID
	// clear errors
	dlerror();

	LoaderHandle = FPlatformProcess::GetDllHandle(TEXT("libopenxr_loader.so"));
	if (LoaderHandle == nullptr)
	{
		UE_LOG(LogXvisioOpenXRPlugin, Error, TEXT("Unable to load libopenxr_loader.so, error %s"), ANSI_TO_TCHAR(dlerror()));
		return false;
	}

	// clear errors
	dlerror();

	PFN_xrGetInstanceProcAddr xrGetInstanceProcAddrPtr = (PFN_xrGetInstanceProcAddr)FPlatformProcess::GetDllExport(LoaderHandle, TEXT("xrGetInstanceProcAddr"));
	if (xrGetInstanceProcAddrPtr == nullptr)
	{
		UE_LOG(LogXvisioOpenXRPlugin, Error, TEXT("Unable to load OpenXR xrGetInstanceProcAddr, error %s"), ANSI_TO_TCHAR(dlerror()));
		return false;
	}
	*OutGetProcAddr = xrGetInstanceProcAddrPtr;

	// PFN_xrCreateHandTrackerEXT xrCreateHandTrackerEXTPtr = (PFN_xrCreateHandTrackerEXT)FPlatformProcess::GetDllExport(LoaderHandle, TEXT("xrCreateHandTrackerEXT"));

	// xrCreateHandTrackerEXTPtr();

	extern struct android_app *GNativeAndroidApp;
	PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR;
	xrGetInstanceProcAddrPtr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction *)&xrInitializeLoaderKHR);
	if (xrInitializeLoaderKHR == nullptr)
	{
		UE_LOG(LogXvisioOpenXRPlugin, Error, TEXT("Unable to load OpenXR xrInitializeLoaderKHR"));
		return false;
	}
	XrLoaderInitInfoAndroidKHR LoaderInitializeInfoAndroid;
	LoaderInitializeInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
	LoaderInitializeInfoAndroid.next = NULL;
	LoaderInitializeInfoAndroid.applicationVM = GNativeAndroidApp->activity->vm;
	LoaderInitializeInfoAndroid.applicationContext = GNativeAndroidApp->activity->clazz;
	XR_ENSURE(xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR *)&LoaderInitializeInfoAndroid));

	UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Loaded Xvisio OpenXR Loader"));
	// clear errors
	/*PFN_xrDeviceRgbSetBrightnessXVISIO xrDeviceRgbSetBrightnessXVISIO;
	xrGetInstanceProcAddrPtr(XR_NULL_HANDLE, "xrDeviceRgbSetBrightnessXVISIO", (PFN_xrVoidFunction *)&xrDeviceRgbSetBrightnessXVISIO);
	if (xrDeviceRgbSetBrightnessXVISIO == nullptr)
	{
		UE_LOG(LogXvisioOpenXRPlugin, Error, TEXT("Unable to load OpenXR xrDeviceRgbSetBrightnessXVISIO"));
		return false;
	}*/
	//XR_ENSURE(xrDeviceRgbSetBrightnessXVISIO(_SessionHandle,1);
	return true;
#else  // PLATFORM_ANDROID
	return false;
#endif // PLATFORM_ANDROID
}
void FXvisioOpenXRHMD::SetSessionHandle(const XrSession &inSession)
{
	_SessionHandle = inSession;
}
void FXvisioOpenXRHMD::StartupModule()
{
	UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("FXvisioOpenXRHMD StartupModule start"));
	RegisterOpenXRExtensionModularFeature();
	//	handTrackingModule.Register();
	g_XvisioOpenXRModule = this;
}
void FXvisioOpenXRHMD::SetInstanceHandle(const XrInstance &inInstance)
{
	_InstanceHandle = inInstance;
}

bool FXvisioOpenXRHMD::IsActive() const
{
	return _bIsFeatureToggled;
}
bool FXvisioOpenXRHMD::IsStandaloneStereoOnlyDevice()
{
	return true;
}

bool FXvisioOpenXRHMD::GetRequiredExtensions(TArray<const ANSICHAR *> &OutExtensions)
{
	OutExtensions.Add(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
	return true;
}

bool FXvisioOpenXRHMD::GetInteractionProfile(XrInstance InInstance, FString &OutKeyPrefix, XrPath &OutPath, bool &OutHasHaptics)
{
	// UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR GetInteractionProfile"));
	return true;
}

bool FXvisioOpenXRHMD::GetSpectatorScreenController(FHeadMountedDisplayBase *InHMDBase, TUniquePtr<FDefaultSpectatorScreenController> &OutSpectatorScreenController)
{
#if PLATFORM_ANDROID
	OutSpectatorScreenController = nullptr;
	return true;
#else  // PLATFORM_ANDROID
	OutSpectatorScreenController = MakeUnique<FDefaultSpectatorScreenController>(InHMDBase);
	return false;
#endif // PLATFORM_ANDROID
}

// void FXvisioOpenXRHMD::AddActions(XrInstance Instance, TFunction<XrAction(XrActionType InActionType, const FName& InName, const TArray<XrPath>& InSubactionPaths)> AddAction)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR AddActions"));
// 	return;
// }

// void FXvisioOpenXRHMD::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
// {
// 	return;
// }

// const void* FXvisioOpenXRHMD::OnCreateInstance(class IOpenXRHMDPlugin* InPlugin, const void* InNext)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnCreateInstance"));
// 	return InNext;
// }

// const void* FXvisioOpenXRHMD::OnGetSystem(XrInstance InInstance, const void* InNext)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnGetSystem"));
// 	return InNext;
// }

const void *FXvisioOpenXRHMD::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void *InNext)
{
	UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnCreateSession"));
#if PLATFORM_ANDROID
	if (GRHISupportsRHIThread && GIsThreadedRendering && GUseRHIThread_InternalUseOnly)
	{
		SetRHIThreadEnabled(false, false);
	}

#endif // PLATFORM_ANDROID

	return InNext;
}

// const void* FXvisioOpenXRHMD::OnBeginSession(XrSession InSession, const void* InNext)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnBeginSession"));
// 	return InNext;
// }

// const void* FXvisioOpenXRHMD::OnBeginFrame(XrSession InSession, XrTime DisplayTime, const void* InNext)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnBeginFrame"));
// 	return InNext;
// }

// const void* FXvisioOpenXRHMD::OnBeginProjectionView(XrSession InSession, int32 InLayerIndex, int32 InViewIndex, const void* InNext)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnBeginProjectionView"));
// 	return InNext;
// }

// const void* FXvisioOpenXRHMD::OnBeginDepthInfo(XrSession InSession, int32 InLayerIndex, int32 InViewIndex, const void* InNext)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnBeginDepthInfo"));
// 	return InNext;
// }

const void *FXvisioOpenXRHMD::OnEndProjectionLayer(XrSession InSession, int32 InLayerIndex, const void *InNext, XrCompositionLayerFlags &OutFlags)
{
	// UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnEndProjectionLayer"));

	// XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT is required right now because the Xvisio mobile runtime blends using alpha otherwise,
	// and we don't have proper inverse alpha support in OpenXR yet (once OpenXR supports inverse alpha, or we change the runtime behavior, remove this)
	OutFlags |= XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
	OutFlags |= XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
	return InNext;
}

FString FXvisioOpenXRHMD::GetDisplayName()
{
	return FString(TEXT("Xvisio XR Runtime"));
}

// const void* FXvisioOpenXRHMD::OnEndFrame(XrSession InSession, XrTime DisplayTime, const TArray<XrSwapchainSubImage> InColorImages, const TArray<XrSwapchainSubImage> InDepthImages, const void* InNext)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnEndFrame"));
// 	return InNext;
// }

// const void* FXvisioOpenXRHMD::OnSyncActions(XrSession InSession, const void* InNext)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR OnSyncActions"));
// 	return InNext;
// }

// void FXvisioOpenXRHMD::PostSyncActions(XrSession InSession)
// {
// 	//UE_LOG(LogXvisioOpenXRPlugin, Log, TEXT("Xvisio OpenXR PostSyncActions"));
// 	return;
// }
IMPLEMENT_MODULE(FXvisioOpenXRHMD, XvisioOpenXR)