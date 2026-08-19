#include <falso_jni/FalsoJNI.h>
#include <falso_jni/FalsoJNI_Impl.h>
#include <falso_jni/FalsoJNI_Logger.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "reimpl/asset_manager.h"
#include "utils/logger.h"
#include "audio.h"

/*
 * JNI Methods
 */

static void stub_void(jmethodID id, va_list args) { (void)id; (void)args; }
static jint stub_int(jmethodID id, va_list args) { (void)id; (void)args; return 0; }
static jboolean stub_bool(jmethodID id, va_list args) { (void)id; (void)args; return JNI_FALSE; }
static jobject stub_obj(jmethodID id, va_list args) { (void)id; (void)args; return NULL; }
static jobject stub_bytearray(jmethodID id, va_list args) {
    (void)id; (void)args;
    return (jobject)jda_alloc(0, FIELD_TYPE_BYTE);
}

static jint getAssetResSize_impl(jmethodID id, va_list args) {
    (void)id;

    jbyteArray jarr = va_arg(args, jbyteArray);
    
    jsize string_len = jni->GetArrayLength(jni, jarr);
    char file_name[256];
    char path_name[256];

    if (string_len <= 0 || string_len >= sizeof(file_name)){
        return 0;
    }

    jbyte *bytes = jni->GetByteArrayElements(jni, jarr, NULL);
    memcpy(file_name, bytes, string_len);
    file_name[string_len] = '\0';

    jni->ReleaseByteArrayElements(jni, jarr, bytes, JNI_ABORT);

    struct stat st;

    sceClibPrintf("[getAssetResSize] INFO: Asset open : %s\n", file_name);

    snprintf(path_name, sizeof(path_name), "%s%s", "ux0:/data/destinia/assets/", file_name);

    sceClibPrintf("[getAssetResSize] INFO: Path open : %s\n", path_name);

    if (stat(path_name, &st) == 0) {
        l_debug("Stat: %s -> size=%d", path_name, (int)st.st_size);
        return (int)st.st_size;
    }

    return 0;
}

static jobject getAssetRes_impl(jmethodID id, va_list args) {
    jbyteArray jarr = va_arg(args, jbyteArray);
    
    jsize string_len = jni->GetArrayLength(jni, jarr);
    char file_name[256];
    char path_name[256];

    if (string_len <= 0 || string_len >= sizeof(file_name)){
        return 0;
    }

    jbyte *bytes = jni->GetByteArrayElements(jni, jarr, NULL);
    memcpy(file_name, bytes, string_len);
    file_name[string_len] = '\0';

    sceClibPrintf("[getAssetRes] INFO: Asset open : %s\n", file_name);

    AAsset* asset = AAssetManager_open(NULL, file_name, AASSET_MODE_BUFFER);
    if (!asset) {
        return NULL;
    }

    int length = AAsset_getLength(asset);
    
    if (length <= 0) {
        AAsset_close(asset);
        return NULL;
    }

    jbyteArray returnjarr = NewByteArray(NULL, length);
    if (!returnjarr) {
        AAsset_close(asset);
        return NULL;
    }

    void* buffer = malloc(length);
    if (buffer) {
        AAsset_read(asset, buffer, length);
        SetByteArrayRegion(NULL, returnjarr, 0, length, (jbyte*)buffer);
        free(buffer);
    }

    AAsset_close(asset);

    return (jobject)returnjarr;
}

static void playSound_impl(jmethodID id, va_list args) {
    jint sndID   = va_arg(args, jint);

    sceClibPrintf("[playSound] sndID=%d \n", sndID);

    audio_play_sound(sndID);
}

static void setSoundVolume_impl(jmethodID id, va_list args) {
    jint sndVol1   = va_arg(args, jint);
    jint sndVol2   = va_arg(args, jint);
    jint sndVol3   = va_arg(args, jint);

    sceClibPrintf("[setSoundVolume] sndVol1=%d sndVol2=%d sndVol3=%d \n", sndVol1, sndVol2, sndVol3);
}

static int logEvent_impl(jmethodID id, va_list args) {
    jbyteArray jarr = va_arg(args, jbyteArray);

    return 0;
}

NameToMethodID nameToMethodId[] = {
    { 100, "getAssetResSize", METHOD_TYPE_INT },
    { 101, "getAssetRes", METHOD_TYPE_OBJECT },
    { 102, "playSound", METHOD_TYPE_VOID },
    { 103, "setSoundVolume", METHOD_TYPE_VOID},
    { 104, "logEvent", METHOD_TYPE_INT}

};

MethodsBoolean methodsBoolean[] = {
    { 100, stub_bool },
    { 101, stub_bool },
    { 102, stub_bool },
    { 103, stub_bool },
    { 104, stub_bool }
};
MethodsByte methodsByte[] = {
    { 100, stub_void },
    { 101, stub_void },
    { 102, stub_void },
    { 103, stub_void },
    { 104, stub_void }
};
MethodsChar methodsChar[] = {
    { 100, stub_void },
    { 101, stub_void },
    { 102, stub_void },
    { 103, stub_void },
    { 104, stub_void }
};
MethodsDouble methodsDouble[] = {
    { 100, stub_int },
    { 101, stub_int },
    { 102, stub_int },
    { 103, stub_void },
    { 104, stub_void }
};
MethodsFloat methodsFloat[] = {
    { 100, stub_int },
    { 101, stub_int },
    { 102, stub_int },
    { 103, stub_int },
    { 104, stub_int }
};
MethodsInt methodsInt[] = {
    { 100, getAssetResSize_impl },
    { 101, stub_int },
    { 102, stub_int },
    { 103, stub_int },
    { 104, logEvent_impl }
};
MethodsLong methodsLong[] = {
    { 100, stub_int },
    { 101, stub_int },
    { 102, stub_void },
    { 103, stub_void },
    { 104, stub_void }
};
MethodsObject methodsObject[] = {
    { 100, stub_obj },
    { 101, getAssetRes_impl },
    { 102, stub_obj },
    { 103, stub_obj },
    { 104, stub_obj }
};
MethodsShort methodsShort[] = {
    { 100, stub_void },
    { 101, stub_void },
    { 102, stub_void },
    { 103, stub_void },
    { 104, stub_void }
};
MethodsVoid methodsVoid[] = {
    { 100, stub_void },
    { 101, stub_void },
    { 102, playSound_impl },
    { 103, playSound_impl },
    { 104, stub_void }
};

/*
 * JNI Fields
 */

// System-wide constant that applications sometimes request
// https://developer.android.com/reference/android/content/Context.html#WINDOW_SERVICE
char WINDOW_SERVICE[] = "window";

// System-wide constant that's often used to determine Android version
// https://developer.android.com/reference/android/os/Build.VERSION.html#SDK_INT
// Possible values: https://developer.android.com/reference/android/os/Build.VERSION_CODES
const int SDK_INT = 19; // Android 4.4 / KitKat

NameToFieldID nameToFieldId[] = {
	{ 0, "WINDOW_SERVICE", FIELD_TYPE_OBJECT },
	{ 1, "SDK_INT", FIELD_TYPE_INT },
};

FieldsBoolean fieldsBoolean[] = {};
FieldsByte fieldsByte[] = {};
FieldsChar fieldsChar[] = {};
FieldsDouble fieldsDouble[] = {};
FieldsFloat fieldsFloat[] = {};
FieldsInt fieldsInt[] = {
	{ 1, SDK_INT },
};
FieldsObject fieldsObject[] = {
	{ 0, WINDOW_SERVICE },
};
FieldsLong fieldsLong[] = {};
FieldsShort fieldsShort[] = {};

__FALSOJNI_IMPL_CONTAINER_SIZES
