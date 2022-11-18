#include <android/log.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

#include "new.hpp"
#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::Option;
using zygisk::ServerSpecializeArgs;

#define TAG ".xlDownload"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

class XlDownloadModule : public zygisk::ModuleBase {
  private:
    Api *api;
    JNIEnv *env;

    static const char *blockPath;
    static bool install;

    static int (*_open)(const char *pathname, int flags, mode_t mode);
    static int open(const char *pathname, int flags, mode_t mode);

    static int (*_mkdir)(const char *pathname, mode_t mode);
    static int (mkdir)(const char *pathname, mode_t mode);

    static int (*_access)(const char *pathname, int mode);
    static int (access)(const char *pathname, int mode);

    static void canonicalPath(char *path);

  public:
    void onLoad(Api *api, JNIEnv *env) override;
    void preAppSpecialize(AppSpecializeArgs *args) override;
    void postAppSpecialize(const AppSpecializeArgs *args) override;
};

const char *XlDownloadModule::blockPath;
bool XlDownloadModule::install;

int (*XlDownloadModule::_open)(const char *pathname, int flags, mode_t mode);
int XlDownloadModule::open(const char *pathname, int flags, mode_t mode) {
    if (strncmp(pathname, blockPath, strlen(blockPath)) == 0) {
        LOGD("redirect open %s to /dev/null", pathname);

        return _open("/dev/null", flags, mode);
    }

    return _open(pathname, flags, mode);
}

int (*XlDownloadModule::_mkdir)(const char *pathname, mode_t mode);
int (XlDownloadModule::mkdir)(const char *pathname, mode_t mode) {
    if (strncmp(pathname, blockPath, strlen(blockPath)) == 0) {
        LOGD("ignore creating directory %s", pathname);

        errno = 0;

        return 0;
    }

    return _mkdir(pathname, mode);
}

int (*XlDownloadModule::_access)(const char *pathname, int mode);
int (XlDownloadModule::access)(const char *pathname, int mode) {
    if (strncmp(pathname, blockPath, strlen(blockPath)) == 0) {
        LOGD("redirect access %s to /dev/null", pathname);

        return _access("/dev/null", mode);
    }

    return _access(pathname, mode);
}

void XlDownloadModule::canonicalPath(char *buf) {
    char *wp = buf;
    int skip = 0;
    for (const char *p = buf; *p != 0; p++) {
        if (*p == '/') {
            if (skip) {
                continue;
            }

            skip = 1;
        } else {
            skip = 0;
        }

        *wp++ = *p;
    }

    *wp = 0;
}

void XlDownloadModule::onLoad(Api *api, JNIEnv *env) {
    this->api = api;
    this->env = env;
}

void XlDownloadModule::preAppSpecialize(AppSpecializeArgs *args) {
    const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
    install = strcmp(process, "android.process.media") == 0;
    env->ReleaseStringUTFChars(args->nice_name, process);

    if (!install) {
        api->setOption(Option::DLCLOSE_MODULE_LIBRARY);
    }
}

void XlDownloadModule::postAppSpecialize(const AppSpecializeArgs *args) {
    if (install) {
        jclass cEnviromnent = env->FindClass("android/os/Environment");
        jmethodID mGetDirectory = env->GetStaticMethodID(cEnviromnent, "getExternalStorageDirectory", "()Ljava/io/File;");
        jclass cFile = env->FindClass("java/io/File");
        jmethodID mAbsolutePath = env->GetMethodID(cFile, "getAbsolutePath", "()Ljava/lang/String;");

        jobject oDirectory = env->CallStaticObjectMethod(cEnviromnent, mGetDirectory);
        jstring sPath = reinterpret_cast<jstring>(env->CallObjectMethod(oDirectory, mAbsolutePath));

        const char *path = env->GetStringUTFChars(sPath, nullptr);
        char buffer[PATH_MAX] = {0};
        snprintf(buffer, sizeof(buffer) - 1, "%s/%s", path, ".xlDownload");
        canonicalPath(buffer);
        blockPath = strdup(buffer);
        env->ReleaseStringUTFChars(sPath, path);

        api->pltHookRegister(".*", "open", reinterpret_cast<void *>(&XlDownloadModule::open), reinterpret_cast<void **>(&XlDownloadModule::_open));
        api->pltHookRegister(".*", "mkdir", reinterpret_cast<void *>(&XlDownloadModule::mkdir), reinterpret_cast<void **>(&XlDownloadModule::_mkdir));
        api->pltHookRegister(".*", "access", reinterpret_cast<void *>(&XlDownloadModule::access), reinterpret_cast<void **>(&XlDownloadModule::_access));
        api->pltHookCommit();

        LOGD("installed blockPath = %s", blockPath);
    }
}

REGISTER_ZYGISK_MODULE(XlDownloadModule)
