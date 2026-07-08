#include "EVPlatformFileExchangeServiceFactory.h"

// #if PLATFORM_ANDROID
////#include "EVAndroidFileExchangeService.h"
// #elif PLATFORM_IOS
////#include "EVIOSFileExchangeService.h"
// #elif WITH_EDITOR || PLATFORM_WINDOWS
////#include "EVEditorFileExchangeService.h"
// #else
// #include "EVUnsupportedFileExchangeService.h"
// #endif
#include "EVUnsupportedFileExchangeService.h"

UObject* UEVPlatformFileExchangeServiceFactory::Create(UObject* Outer)
{
    UObject* SafeOuter = IsValid(Outer) ? Outer : GetTransientPackage();

    // #if PLATFORM_ANDROID
    //     //return NewObject<UEVAndroidFileExchangeService>(SafeOuter);
    // #elif PLATFORM_IOS
    //     //return NewObject<UEVIOSFileExchangeService>(SafeOuter);
    // #elif WITH_EDITOR || PLATFORM_WINDOWS
    //     //return NewObject<UEVEditorFileExchangeService>(SafeOuter);
    // #else
    //     return NewObject<UEVUnsupportedFileExchangeService>(SafeOuter);
    // #endif
    return NewObject<UEVUnsupportedFileExchangeService>(SafeOuter);
}