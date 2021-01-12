#include "drv.h"

//
// We support unload (but deny it)
//
const UINT8 _gDriverUnloadImageCount = 1;

//
// We require at least UEFI 2.0
//
const UINT32 _gUefiDriverRevision = 0x200;
const UINT32 _gDxeRevision = 0x200;

//
// Our name
//
CHAR8 *gEfiCallerBaseName = "HookingDriver";
EFI_STATUS (*origAddress)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*, CHAR16*) = NULL;

EFI_STATUS
EFIAPI
UefiUnload (
    IN EFI_HANDLE ImageHandle
    )
{
    //
    // Do not allow unload
    //
    return EFI_ACCESS_DENIED;
}

EFI_STATUS
EFIAPI
RandomStuff(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        *This,
    IN CHAR16                                 *String
    )
{
    // gST->ConOut->OutputString(This, String);


    CHAR16* MyString = L"No way you can print\r\n";
    origAddress(This, MyString);

    return 0;
}

EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
    EFI_STATUS efiStatus;

    //
    // Install required driver binding components
    //
    efiStatus = EfiLibInstallDriverBindingComponentName2(ImageHandle,
                                                         SystemTable,
                                                         &gDriverBindingProtocol,
                                                         ImageHandle,
                                                         &gComponentNameProtocol,
                                                         &gComponentName2Protocol);


    CHAR16* MyString = L"I have written my first UEFI driver\r\n";

    gST->ConOut->OutputString(gST->ConOut, MyString);

    origAddress = gST->ConOut->OutputString;
    gST->ConOut->OutputString = RandomStuff;

    RandomStuff(gST->ConOut, MyString);

    return efiStatus;
}

