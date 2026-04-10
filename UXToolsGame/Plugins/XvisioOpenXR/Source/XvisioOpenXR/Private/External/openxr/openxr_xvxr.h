#ifndef OPENXR_XVXR_H_
#define OPENXR_XVXR_H_ 1

/*
** Copyright (c) 2017-2022, The Khronos Group Inc.
**
** SPDX-License-Identifier: Apache-2.0 OR MIT
*/

/*
** This header is generated from the Khronos OpenXR XML API Registry.
**
*/
#include "openxr.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define XR_XVISIO_api 1
#define XR_XVISIO_api_SPEC_VERSION 1
#define XR_XVISIO_API_EXTENSION_NAME "XR_XVISIO_api"
#define XR_XVISIO_HAND_JOINT_COUNT 50

    typedef enum XrPlaneSourceXVISIO
    {
        XR_PLANE_SOURCE_TOF_XVISIO = 0,
        XR_PLANE_SOURCE_STEREO_XVISIO = 1,
        XR_PLANE_SOURCE_TOF_NOSURFACE_XVISIO = 2,
        XR_PLANE_SOURCE_MAX_ENUM_XVISIO = 0x7FFFFFFF
    } XrPlaneSourceXVISIO;
    typedef struct XvSkeletonXVISIO
    {
        XrPosef handPose[XR_XVISIO_HAND_JOINT_COUNT];
        uint32_t poseSize;
    } XvSkeletonXVISIO;

    typedef struct XrObjectDataXVISIO
    {
        XrVector3f point;
        uint32_t blobIndex;
        uint32_t typeID;
        const char *typeName;
        double confidence;
        double x;
        double y;
        double width;
        double height;
        uint32_t pointsSize;
        XrVector3f *keypoints;
    } XrObjectDataXVISIO;

    typedef struct XrGazePointXVISIO
    {
        uint32_t gazeBitMask;
        XrVector3f gazePoint;
        XrVector3f rawPoint;
        XrVector3f smoothPoint;
        XrVector3f gazeOrigin;
        XrVector3f gazeDirection;
        float re;
        uint32_t exDataBitMask;
    } XrGazePointXVISIO;

    typedef struct XrGazePupilINFOXVISIO
    {
        uint32_t pupilBitMask;
        XrVector2f pupilCenter;
        float pupilDistance;
        float pupilDiameter;
        float pupilDiameterMM;
        float pupilMinorAxis;
        float pupilMinorAxisMM;
    } XrGazePupilINFOXVISIO;

    typedef struct XrGazeDataXVISIO
    {
        unsigned long long timestamp;
        uint32_t recommend;
        XrGazePointXVISIO gazePoints[2];
        XrGazePupilINFOXVISIO pupilInfo[2];
    } XrGazeDataXVISIO;

    typedef struct XrXplanPackageXVISIO
    {
        uint32_t points_nb;
        char idStr[32];
        XrVector3f normal;
        uint32_t verticesSize;
        XrVector3f *vertices;
        uint32_t trianglesSize;
        XrVector3f *triangles;
        double distance;
    } XrXplanPackageXVISIO;

    typedef struct XrSurfaceDataXVISIO
    {
        uint32_t mapId;
        uint32_t version;
        uint32_t id;
        uint32_t verticesSize;
        XrVector3f *vertices;
        XrVector3f *vertexNormals;
        uint32_t trianglesSize;
        XrVector3f *triangles;
        uint32_t textureWidth;
        uint32_t textureHeight;
    } XrSurfaceDataXVISIO;

    typedef XrResult(XRAPI_PTR *PFN_xrCameraDistance)(XrSession session, uint32_t distance);
    typedef XrResult(XRAPI_PTR *PFN_xrDeviceRgbSetBrightnessXVISIO)(XrSession session, uint32_t brightness);
    typedef XrResult(XRAPI_PTR *PFN_xrGetHandTrackingDataXVISIO)(XrSession session, XvSkeletonXVISIO *skeleton);
    typedef XrResult(XRAPI_PTR *PFN_xrStartHandTrackingXVISIO)(XrSession session, uint32_t type);
    typedef XrResult(XRAPI_PTR *PFN_xrStartRgbStreamingXVISIO)(XrSession session, uint32_t solution);
    typedef XrResult(XRAPI_PTR *PFN_xrGetRgbStreamXVISIO)(XrSession session, unsigned char *data, uint32_t type, uint32_t width, uint32_t height, double *timestamp);
    typedef XrResult(XRAPI_PTR *PFN_xrStartObjectTrackingXVISIO)(XrSession session, char *modelPath, char *descriptorPath);
    typedef XrResult(XRAPI_PTR *PFN_xrGetObjectTrackingDataXVISIO)(XrSession session, uint32_t *objNum, XrObjectDataXVISIO *objectDatas);
    typedef XrResult(XRAPI_PTR *PFN_xrStartDetectPlaneXVISIO)(XrSession session, XrPlaneSourceXVISIO sourceType);
    typedef XrResult(XRAPI_PTR *PFN_xrGetPlaneDataXVISIO)(XrSession session, XrXplanPackageXVISIO *data, uint32_t *size);
    typedef XrResult(XRAPI_PTR *PFN_xrStartSlamMapXVISIO)(XrSession session);
    typedef XrResult(XRAPI_PTR *PFN_xrLoadMapAndSwitchToCslamXVISIO)(XrSession session, char *mapPath);
    typedef XrResult(XRAPI_PTR *PFN_xrSaveMapAndSwitchToCslamXVISIO)(XrSession session, char *mapPath);
    typedef XrResult(XRAPI_PTR *PFN_xrStartGetSurfaceXVISIO)(XrSession session);
    typedef XrResult(XRAPI_PTR *PFN_xrGetSurfaceDataXVISIO)(XrSession session, XrSurfaceDataXVISIO *data, uint32_t *size);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
    XRAPI_ATTR XrResult XRAPI_CALL xrCameraDistance(
        XrSession session,
        uint32_t distance);

    XRAPI_ATTR XrResult XRAPI_CALL xrDeviceRgbSetBrightnessXVISIO(
        XrSession session,
        uint32_t brightness);

    XRAPI_ATTR XrResult XRAPI_CALL xrGetHandTrackingDataXVISIO(
        XrSession session,
        XvSkeletonXVISIO *skeleton);

    XRAPI_ATTR XrResult XRAPI_CALL xrStartHandTrackingXVISIO(
        XrSession session,
        uint32_t type);

    XRAPI_ATTR XrResult XRAPI_CALL xrStartRgbStreamingXVISIO(
        XrSession session,
        uint32_t solution);

    XRAPI_ATTR XrResult XRAPI_CALL xrGetRgbStreamXVISIO(
        XrSession session,
        unsigned char *data,
        uint32_t type,
        uint32_t width,
        uint32_t height,
        double *timestamp);

    XRAPI_ATTR XrResult XRAPI_CALL xrStartObjectTrackingXVISIO(
        XrSession session,
        char *modelPath,
        char *descriptorPath);

    XRAPI_ATTR XrResult XRAPI_CALL xrGetObjectTrackingDataXVISIO(
        XrSession session,
        uint32_t *objNum,
        XrObjectDataXVISIO *objectDatas);

    XRAPI_ATTR XrResult XRAPI_CALL xrStartDetectPlaneXVISIO(
        XrSession session,
        XrPlaneSourceXVISIO sourceType);

    XRAPI_ATTR XrResult XRAPI_CALL xrGetPlaneDataXVISIO(
        XrSession session,
        XrXplanPackageXVISIO *data,
        uint32_t *size);

    XRAPI_ATTR XrResult XRAPI_CALL xrStartSlamMapXVISIO(
        XrSession session);

    XRAPI_ATTR XrResult XRAPI_CALL xrLoadMapAndSwitchToCslamXVISIO(
        XrSession session,
        char *mapPath);

    XRAPI_ATTR XrResult XRAPI_CALL xrSaveMapAndSwitchToCslamXVISIO(
        XrSession session,
        char *mapPath);

    XRAPI_ATTR XrResult XRAPI_CALL xrStartGetSurfaceXVISIO(
        XrSession session);

    XRAPI_ATTR XrResult XRAPI_CALL xrGetSurfaceDataXVISIO(
        XrSession session,
        XrSurfaceDataXVISIO *data,
        uint32_t *size);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */

#define XR_XVISIO_hand_tracking 1
#define XR_XVISIO_hand_tracking_SPEC_VERSION 1
#define XR_XVISIO_HAND_TRACKING_EXTENSION_NAME "XR_XVISIO_hand_tracking"

#ifdef __cplusplus
}
#endif

#endif
