#include "EVPlatformFileExchangeServiceFactory.h"

#if PLATFORM_ANDROID
#include "EVAndroidFileExchangeService.h"
#elif WITH_EDITOR || PLATFORM_WINDOWS
#include "EVEditorFileExchangeService.h"
#else
#include "EVUnsupportedFileExchangeService.h"
#endif

UObject* UEVPlatformFileExchangeServiceFactory::Create(UObject* Outer)
{
    UObject* SafeOuter = IsValid(Outer) ? Outer : GetTransientPackage();

#if PLATFORM_ANDROID
    return NewObject<UEVAndroidFileExchangeService>(SafeOuter);
#elif WITH_EDITOR || PLATFORM_WINDOWS
    return NewObject<UEVEditorFileExchangeService>(SafeOuter);
#else
    return NewObject<UEVUnsupportedFileExchangeService>(SafeOuter);
#endif
}