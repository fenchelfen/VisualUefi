#include "drv.h"

#include <IndustryStandard/Mbr.h>
#include <Protocol/BlockIo.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>

//
// We support unload (but dey it)
//
const UINT8 _gDriverUnloadImageCount = 1;

EFI_GUID gEfiBlockIoProtocolGuid = EFI_BLOCK_IO_PROTOCOL_GUID;

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
EFI_BLOCK_READ blockIoOrigAddress;

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
AnotherRandomStuff(
    IN EFI_BLOCK_IO_PROTOCOL* This,
    IN UINT32                 MediaId,
    IN EFI_LBA                Lba,
    IN UINTN                  BufferSize,
    OUT VOID*                 Buffer
)
{
    CHAR16* MyString = L"No way you can print\r\n";
    gST->ConOut->OutputString(gST->ConOut, MyString);

    //blockIoOrigAddress(This, MediaId, Lba, BufferSize, Buffer);

    return 0;
}


EFI_STATUS
EFIAPI
RandomStuff(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        *This,
    IN CHAR16                                 *String
    )
{
    CHAR16 * MyString = L"No way you can print\r\n";
    origAddress(This, MyString);

    return 0;
}

EFI_STATUS
AppPrintBuffer(
    UINT16* Buffer
)
{
    UINTN   i;

    for (i = 0; i <= 0xFF; i++) {
        if ((i % 10) == 0) {
            if (i != 0);
            Print(L"\n");
            Print(L"%.3d: ", i);
        }

        Print(L"%.4X ", Buffer[i]);
    }
    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ReadGpt(
)
{
    EFI_STATUS                  Status;
    EFI_BLOCK_IO_PROTOCOL* BlkIo;
    EFI_HANDLE* BlkIoHandle;
    UINTN                       NoBlkIoHandles;

    //
    // Locate Handles that support BlockIo protocol
    //
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiBlockIoProtocolGuid,
        NULL,
        &NoBlkIoHandles,
        &BlkIoHandle
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    for (UINTN Index = 0; Index < NoBlkIoHandles; Index++) {
        Status = gBS->HandleProtocol(
            BlkIoHandle[Index],
            &gEfiBlockIoProtocolGuid,
            (VOID**)&BlkIo
        );

		blockIoOrigAddress = BlkIo->ReadBlocks;
		BlkIo->ReadBlocks = AnotherRandomStuff;
    }

    return Status;
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


    //CHAR16* MyString = L"I have written my first UEFI driver\r\n";

    //gST->ConOut->OutputString(gST->ConOut, MyString);

    //origAddress = gST->ConOut->OutputString;
    //gST->ConOut->OutputString = RandomStuff;

    //RandomStuff(gST->ConOut, MyString);

    ReadGpt();

    return efiStatus;
}
 