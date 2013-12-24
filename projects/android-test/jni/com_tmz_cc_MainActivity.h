#include <jni.h>

#ifndef _Included_com_tmz_cc_MainActivity
#define _Included_com_tmz_cc_MainActivity
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jboolean JNICALL Java_com_tmz_cc_MainActivity_NativeInit(JNIEnv * env,jclass);
JNIEXPORT jint JNICALL Java_com_tmz_cc_MainActivity_test1(JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
