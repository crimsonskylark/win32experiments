#include <ntddk.h>

extern "C" {

#pragma region WorkItem

struct WorkItemCtx {
  int work_id;
  PIO_WORKITEM work_item;
};

void WorkItem(_In_ PDEVICE_OBJECT DeviceObject, _In_ PVOID Context) {
  auto tid = HandleToULong(PsGetCurrentThreadId());
  auto ctx = static_cast<WorkItemCtx *>(Context);
  if (ctx != nullptr) {
    DbgPrint("[%lu] work item %d is being executed.\n", tid, ctx->work_id);
    DbgPrint("[%lu] device object: %p\n", tid, DeviceObject);
    IoFreeWorkItem(ctx->work_item);
    ExFreePool(ctx, 'KROW');
  }
}

void WorkItemExperiments(PDEVICE_OBJECT DeviceObject) {
  int work_ids[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
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

#pragma end region

NTSTATUS DriverIoctl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
  UNREFERENCED_PARAMETER(DeviceObject);
  UNREFERENCED_PARAMETER(Irp);

  return STATUS_SUCCESS;
}

void DriverUnload(PDRIVER_OBJECT DriverObject) {
  UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(L"\\??\\ExperimentsDriver");
  UNICODE_STRING deviceName =
      RTL_CONSTANT_STRING(L"\\Device\\ExperimentsDriver");

  IoDeleteSymbolicLink(&symLinkName);
  IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, UNICODE_STRING RegistryPath) {
  UNREFERENCED_PARAMETER(RegistryPath);

  auto status = STATUS_SUCCESS;
  auto symLinkCreated = false;

  LARGE_INTEGER start{0};
  LARGE_INTEGER end{0};
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

  WorkItemExperiments(deviceObj);

  return status;

cleanup:
  if (symLinkCreated)
    IoDeleteSymbolicLink(&symLinkName);
  if (deviceObj)
    IoDeleteDevice(deviceObj);
  return status;
}
}