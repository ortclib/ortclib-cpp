#include <jni.h>
#include <stdio.h>
#include "TestMediaEngine.h"

extern "C" {

jint
Java_com_tmz_cc_MainActivity_test1( JNIEnv*  env,
jobject  thiz)
{
printf("!!!! Before TestMediaEngineFactoryPtr\n");
ortc::test::TestMediaEngineFactoryPtr overrideFactory(new ortc::test::TestMediaEngineFactory);

ortc::internal::Factory::override(overrideFactory);
printf("!!!! After TestMediaEngineFactoryPtr\n");

ortc::internal::IMediaEnginePtr mediaEngine = ortc::internal::IMediaEngine::singleton();

printf("!!!! Before setFaceDetection\n");
mediaEngine->setFaceDetection(1,true);
printf("!!!! After setFaceDetection\n");

jint temp = 0;
return temp;
}

jint
Java_com_tmz_cc_MainActivity_test2( JNIEnv*  env,
jobject  thiz)
{
jint temp = 0;
return temp;
}

}


