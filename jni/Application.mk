APP_OPTIM      := release
APP_ABI        := armeabi-v7a arm64-v8a x86 x86_64
APP_CPPFLAGS   := -std=c++17 -Werror=format -fdata-sections -ffunction-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -Oz -fvisibility=hidden -fvisibility-inlines-hidden
APP_LDFLAGS    := -ffixed-x18 -Wl,--hash-style=both -Wl,-exclude-libs,ALL -Wl,--gc-sections
APP_STRIP_MODE := --strip-all --remove-section=.comment
APP_STL        := none
APP_PLATFORM   := android-21
