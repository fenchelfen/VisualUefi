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
    UINT32                      BlockSize;
    UINTN                       Index;
    UINTN                       NoBlkIoHandles;
    EFI_DEVICE_PATH_PROTOCOL* DevPath;
    CHAR16* DevPathString;
    EFI_PARTITION_TABLE_HEADER* PartHdr;
    MASTER_BOOT_RECORD* PMBR;

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

    for (Index = 0; Index < NoBlkIoHandles; Index++) {
        Status = gBS->HandleProtocol(
            BlkIoHandle[Index],
            &gEfiBlockIoProtocolGuid,
            (VOID**)&BlkIo
        );
        if (EFI_ERROR(Status)) {
            continue;
        }
        if (BlkIo->Media->LogicalPartition) {  //if partition skip
            continue;
        }
        DevPath = DevicePathFromHandle(BlkIoHandle[Index]);
        if (DevPath == NULL) {
            continue;
        }
        DevPathString = ConvertDevicePathToText(DevPath, TRUE, FALSE);
        Print(L"%s \nMedia Id: %d, device type: %x, SubType: %x, logical: %x\n", \
            DevPathString, BlkIo->Media->MediaId, DevPath->Type, DevPath->SubType, \
            BlkIo->Media->LogicalPartition);

        BlockSize = BlkIo->Media->BlockSize;
        PartHdr = AllocateZeroPool(BlockSize);
        PMBR = AllocateZeroPool(BlockSize);
        //read LBA0
        Status = BlkIo->ReadBlocks(
            BlkIo,
            BlkIo->Media->MediaId,
            (EFI_LBA)0,							//LBA 0, MBR/Protetive MBR
            BlockSize,
            PMBR
        );
        //read LBA1
        Status = BlkIo->ReadBlocks(
            BlkIo,
            BlkIo->Media->MediaId,
            (EFI_LBA)1,							//LBA 1, GPT
            BlockSize,
            PartHdr
        );
        //check if GPT
        if (PartHdr->Header.Signature == EFI_PTAB_HEADER_ID) {

            if (PMBR->Signature == MBR_SIGNATURE) {
                Print(L"Found Protetive MBR.\n");
            }
            Print(L"LBA 1,");
            Print(L"GPT:\n");
            //
            //you can add some parse GPT data structure here
            //
            AppPrintBuffer((UINT16*)PartHdr);
            Print(L"\n");
        }
        else {
            if (PMBR->Signature == MBR_SIGNATURE) {
                Print(L"LBA 0,");
                Print(L"MBR:\n");
                AppPrintBuffer((UINT16*)PMBR);
                Print(L"\n");
            }
        }

        FreePool(PartHdr);
        FreePool(PMBR);
    }

    //debug dump device path
    //DumpDevicePath();
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
 