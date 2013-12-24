#include <jni.h>
#include <stdio.h>
#include "com_tmz_cc_MainActivity.h"
#include "TestMediaEngine.h"
const char *kInterfacePath = "com/tmz/cc";
static jobject gInterfaceObject, gDataObject;
JNIEnv *genv = NULL;
static JavaVM *gJavaVM;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv *env;
	gJavaVM = vm;

	printf("\n\nJNI_OnLoad called\n\n");
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) 
	{
		printf("Failed to get the environment using GetEnv()");
		return -1;
	}
	printf("Passed to get the environment using GetEnv()\n");
	
	return JNI_VERSION_1_4;
}


JNIEXPORT jboolean JNICALL Java_com_tmz_cc_MainActivity_NativeInit(JNIEnv * env,jclass)
{
    return true;
}

JNIEXPORT jint JNICALL Java_com_tmz_cc_MainActivity_test1(JNIEnv *, jclass)
{
	//doMediaEngineTest();
	printf("!!!! Before TestMediaEngineFactoryPtr\n");
	ortc::test::TestMediaEngineFactoryPtr overrideFactory(new ortc::test::TestMediaEngineFactory);

	ortc::internal::Factory::override(overrideFactory);
	printf("!!!! After TestMediaEngineFactoryPtr\n");

	ortc::internal::IMediaEnginePtr mediaEngine = ortc::internal::IMediaEngine::singleton();

	printf("!!!! Before setFaceDetection\n");
	mediaEngine->setFaceDetection(1,true);
	printf("!!!! After setFaceDetection\n");

	jint val = mediaEngine->getCameraType(1);

	jint temp = 5;
	return val;
}



