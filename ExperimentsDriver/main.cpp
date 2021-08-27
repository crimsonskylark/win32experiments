#include <ntddk.h>
#include <aux_klib.h>


extern "C" {

#pragma region WorkItem

    struct WorkItemCtx {
        int work_id;
        PIO_WORKITEM work_item;
    };

    void WorkItem(_In_ PDEVICE_OBJECT DeviceObject, _In_ PVOID Context) {
        auto tid = HandleToULong(PsGetCurrentThreadId());
        auto ctx = static_cast<WorkItemCtx*>(Context);
        if (ctx != nullptr) {
            DbgPrint("[%lu] context pointer: %p\n", tid, ctx);
            DbgPrint("[%lu] work item %d is being executed.\n", tid, ctx->work_id);
            DbgPrint("[%lu] device object: %p\n", tid, DeviceObject);
            IoFreeWorkItem(ctx->work_item);
            ExFreePoolWithTag(ctx, 'KROW');
        }
    }

    void WorkItemExperiments(PDEVICE_OBJECT DeviceObject) {
        int work_ids[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        for (auto id : work_ids) {
            auto work_item = IoAllocateWorkItem(DeviceObject);
            if (!work_item) {
                DbgPrint("unable to allocate work item: out of memory\n");
                return;
            }
            auto ctx = static_cast<WorkItemCtx*>(
                ExAllocatePoolWithTag(PagedPool, sizeof(WorkItemCtx), 'KROW'));
            if (ctx) {
                ctx->work_id = id;
                ctx->work_item = work_item;
                DbgPrint("[main thread] queuing work item %d (%p)\n", ctx->work_id,
                    ctx->work_item);
                IoQueueWorkItem(ctx->work_item, WorkItem, NormalWorkQueue, ctx);
            }
        }
    }

#pragma endregion WorkItem

#pragma region ProcessNotification

    void ProcessNotificationCallback(
        _Inout_ PEPROCESS Process, _In_ HANDLE ProcessId,
        _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo) {
        UNREFERENCED_PARAMETER(Process);
        if (CreateInfo) { /* process is being created */
            DbgPrint("process with pid %lu is being created\n",
                HandleToULong(ProcessId));
            const char* pinfo = "image: %S\nsize: %lu\n";
            DbgPrint(pinfo,
                CreateInfo->FileOpenNameAvailable
                ? CreateInfo->ImageFileName->Buffer
                : L"N/A",
                CreateInfo->Size);
        }
        else { /* process is being destroyed */
            DbgPrint("process with pid %lu is being destroyed\n",
                HandleToULong(ProcessId));
        }
    }

    void SetupNotification() {
        auto status =
            PsSetCreateProcessNotifyRoutineEx(ProcessNotificationCallback, FALSE);
        if (!NT_SUCCESS(status)) {
            DbgPrint("unable to register for process notifications: 0x%08x\n", status);
            return;
        }
        // :-)
    }

#pragma endregion ProcessNotification

#pragma region DPC

    VOID DpcCb(struct _KDPC* Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2) {
        UNREFERENCED_PARAMETER(SystemArgument1);
        UNREFERENCED_PARAMETER(SystemArgument2);
        (VOID)DeferredContext;
        (VOID)Dpc;
        DbgPrint("DPC was called, IRQL: %c\n", KeGetCurrentIrql());
    }

    VOID DpcExperiment() {
        KDPC dpc{};
        KeInitializeDpc(&dpc, DpcCb, nullptr);
        KeInsertQueueDpc(&dpc, nullptr, nullptr);
    }

#pragma endregion DPC

#pragma region ExecutiveResource

    ERESOURCE Res = { };
    KEVENT Event = { };
    ULONG SharedVariable = 0;
    LARGE_INTEGER Interval = { };

    VOID TestExecutiveResourceConcurrentRead() {
        auto threadid = HandleToULong(PsGetCurrentThreadId());
        for (;;) {
            DbgPrint("[%lu] Entering critical region", threadid);
            KeEnterCriticalRegion();
            ExAcquireResourceSharedLite(&Res, TRUE);
            DbgPrint("[%lu] Shared variable value: %lu", threadid, SharedVariable);
            ExReleaseResourceLite(&Res);
            KeLeaveCriticalRegion();
            KeDelayExecutionThread(KernelMode, FALSE, &Interval);
        }
    }

    NTSTATUS InitExecutiveResource() {
        auto status = ExInitializeResourceLite(&Res);
        if (!NT_SUCCESS(status)) {
            DbgPrint("Failed to initialize executive resource: %08x\n", status);
            return status;
        }
        Interval.QuadPart = -50000000I64;
        DbgPrint("Initialized executive resource, location: %p\n", &Res);
        return status;
    }

#pragma endregion ExecutiveResource


#pragma region SMBios

    struct SMBiosHeader {
        CHAR  Used20CallingMethod;
        CHAR  SMBIOSMajorVersion;
        CHAR  SMBIOSMinorVersion;
        CHAR  DmiRevision;
        ULONG  Length;
        CHAR SMBiosTableData[];
    };

    void GetSMBiosInformation() {
        NTSTATUS status = STATUS_SUCCESS;

        AuxKlibInitialize();

        SMBiosHeader* buffer = nullptr;
        ULONG bufferSz = 0;
        ULONG neededBufferSz = 0;

        /* Query struct size */
        status = AuxKlibGetSystemFirmwareTable('RSMB', 0, nullptr, bufferSz, &neededBufferSz);

        DbgPrint("Queried SMBios raw table: %08x | size: %lu\n", status, neededBufferSz);

        if (status == STATUS_BUFFER_TOO_SMALL && neededBufferSz > 0) {
            buffer = static_cast<SMBiosHeader*>(ExAllocatePoolWithTag(PagedPool, neededBufferSz, 'SOIB'));
            if (!buffer) {
                DbgPrint("Unable to allocate memory\n");
                return;
            }

            status = AuxKlibGetSystemFirmwareTable('RSMB', 0, buffer, neededBufferSz, 0);

            if (NT_SUCCESS(status)) {
                DbgPrint("Got SMBios raw table\nMajor version: %d\nMinor version: %d\nLength: %lu\n", buffer->SMBIOSMajorVersion, buffer->SMBIOSMinorVersion, buffer->Length);
            }

        }

        if (buffer)
            ExFreePoolWithTag(buffer, 'SOIB');
    }

#pragma endregion SMBios

    NTSTATUS DriverIoctl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
        UNREFERENCED_PARAMETER(DeviceObject);
        UNREFERENCED_PARAMETER(Irp);

        return STATUS_SUCCESS;
    }

    void DriverUnload(PDRIVER_OBJECT DriverObject) {
        UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(L"\\??\\ExperimentsDriver");
        UNICODE_STRING deviceName =
            RTL_CONSTANT_STRING(L"\\Device\\ExperimentsDriver");

        PsSetCreateProcessNotifyRoutineEx(ProcessNotificationCallback, TRUE);

        IoDeleteSymbolicLink(&symLinkName);
        IoDeleteDevice(DriverObject->DeviceObject);
    }

    NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, UNICODE_STRING RegistryPath) {
        UNREFERENCED_PARAMETER(RegistryPath);

        auto status = STATUS_SUCCESS;
        auto symLinkCreated = false;

        LARGE_INTEGER start{ 0 };
        LARGE_INTEGER end{ 0 };
        KeQuerySystemTimePrecise(&start);

        UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(L"\\??\\ExperimentsDriver");
        UNICODE_STRING deviceName =
            RTL_CONSTANT_STRING(L"\\Device\\ExperimentsDriver");

        PDEVICE_OBJECT deviceObj = nullptr;

        status =
            IoCreateDevice(DriverObject, 0, &deviceName, 0, 0, FALSE, &deviceObj);

        if (!NT_SUCCESS(status)) {
            DbgPrint("unable to create device: 0x%08x\n", status);
            goto cleanup;
        }

        DbgPrint("created device: %p\n", deviceObj);

        status = IoCreateSymbolicLink(&symLinkName, &deviceName);

        if (!NT_SUCCESS(status)) {
            DbgPrint("unable to create symlink to device; 0x%08x\n", status);
            symLinkCreated = false;
            goto cleanup;
        }

        for (size_t idx = 0; idx <= IRP_MJ_MAXIMUM_FUNCTION; idx++) {
            DriverObject->MajorFunction[idx] = nullptr;
        }

        DriverObject->DriverUnload = DriverUnload;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverIoctl;

        KeQuerySystemTimePrecise(&end);

        DbgPrint("took %llu/s to finish driver initialization\n",
            (end.QuadPart - start.QuadPart) / 60);

        /*WorkItemExperiments(deviceObj);
        SetupNotification();*/

        GetSMBiosInformation();

        DpcExperiment();

        return status;

    cleanup:
        if (symLinkCreated)
            IoDeleteSymbolicLink(&symLinkName);
        if (deviceObj)
            IoDeleteDevice(deviceObj);
        return status;
    }
}