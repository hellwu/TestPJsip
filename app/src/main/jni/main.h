#ifndef __H_MAIN_H__
#define __H_MAIN_H__

#include <pjsip/sip_types.h>
#include <android/log.h>

#define TAG "hellw"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

/**
* Memory size to use in caching pool.
* Default: 2MB
*/
#ifndef PJSIP_TEST_MEM_SIZE
#  define PJSIP_TEST_MEM_SIZE	    (2*1024*1024)
#endif

/**
 * 初始化sip
 * @return
 */
int init_pjsip();
/**
 * 注册到sip服务器
 * @return
 */
int regc();
/**
 * 设置pjsip日志
 * @return
 */
int setlog();

/**
 * 测试
 */
void test();

#endif // !__H_MAIN_H__
