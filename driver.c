#include <ddk/wdm.h>
#include <stdio.h>

#define IA32_THERM_STATUS               0x19c
#define MSR_TEMPERATURE_TARGET          0x1a2

struct
{
	struct
	{
		unsigned char Chr[4];
	} AX, BX, CX, DX;
} Brand;

typedef struct
{
	union
	{
		struct
		{
			unsigned int
				StatusBit       :  1-0,
				StatusLog       :  2-1,
				PROCHOT         :  3-2,
				PROCHOTLog      :  4-3,
				CriticalTemp    :  5-4,
				CriticalTempLog :  6-5,
				Threshold1      :  7-6,
				Threshold1Log   :  8-7,
				Threshold2      :  9-8,
				Threshold2Log   : 10-9,
				PowerLimit      : 11-10,
				PowerLimitLog   : 12-11,
				ReservedBits1   : 16-12,
				DTS             : 23-16,
				ReservedBits2   : 27-23,
				Resolution      : 31-27,
				ReadingValid    : 32-31;
		};
			unsigned int Lo     : 32-0;
	};
			unsigned int Hi     : 32-0;
} THERM_STATUS;

typedef struct
{
	union
	{
		struct
		{
				unsigned int
				ReservedBits1   : 16-0,
				Target          : 24-16,
				ReservedBits2   : 32-24;
		};
				unsigned int Lo : 32-0;
	};
				unsigned int Hi : 32-0;
} TJMAX;

typedef struct
{
		int				cpu;
		HANDLE			TID;

		int				Temp;
		TJMAX			TjMax;
		THERM_STATUS	ThermStat;
} CORE;

CORE Core[64];

NTSTATUS NTAPI DriverDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	return STATUS_SUCCESS;
}

VOID NTAPI DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	DbgPrint("WinMSR Unload\n");
	return;
}


VOID ThreadEntry(IN PVOID pArg)
{
	CORE *pCore=(CORE *) pArg;

	KAFFINITY AllCoreBitmap=KeQueryActiveProcessors();
	KAFFINITY ThisCoreBitmap=AllCoreBitmap & (1 << pCore->cpu);
	KeSetSystemAffinityThreadEx(ThisCoreBitmap);

	__asm__ volatile
	(
		"rdmsr ;"
		: "=a" (pCore->TjMax.Lo),
		  "=d" (pCore->TjMax.Hi)
		: "c" (MSR_TEMPERATURE_TARGET)
	);

	__asm__ volatile
	(
		"rdmsr ;"
		: "=a" (pCore->ThermStat.Lo),
		  "=d" (pCore->ThermStat.Hi)
		: "c" (IA32_THERM_STATUS)
	);

	pCore->Temp=pCore->TjMax.Target - pCore->ThermStat.DTS;

	char  DbgStr[32];
	sprintf(DbgStr, "WinMSR: Core(%02d) @ %d°C\n", pCore->cpu, pCore->Temp);
	DbgPrint(DbgStr);
}


NTSTATUS NTAPI DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	DriverObject->DriverUnload = DriverUnload;

	char tmpString[48+1]={0x20}, BrandString[48+1];
	int ix=0, jx=0, px=0;
	for(ix=0; ix<3; ix++)
	{
		__asm__ volatile
		(
			"cpuid ;"
			: "=a"  (Brand.AX),
			  "=b"  (Brand.BX),
			  "=c"  (Brand.CX),
			  "=d"  (Brand.DX)
			: "a"   (0x80000002 + ix)
		);
		for(jx=0; jx<4; jx++, px++)
			tmpString[px]=Brand.AX.Chr[jx];
		for(jx=0; jx<4; jx++, px++)
			tmpString[px]=Brand.BX.Chr[jx];
		for(jx=0; jx<4; jx++, px++)
			tmpString[px]=Brand.CX.Chr[jx];
		for(jx=0; jx<4; jx++, px++)
			tmpString[px]=Brand.DX.Chr[jx];
	}
	for(ix=jx=0; jx < px; jx++)
		if(!(tmpString[jx] == 0x20 && tmpString[jx+1] == 0x20))
			BrandString[ix++]=tmpString[jx];

	DbgPrint(BrandString);

	int cpu;
	for(cpu=0; cpu < KeQueryActiveProcessorCount(NULL); cpu++)
	{
		Core[cpu].cpu=cpu;
		PsCreateSystemThread(&Core[cpu].TID, THREAD_ALL_ACCESS, NULL, NULL, NULL, (PKSTART_ROUTINE) ThreadEntry, (PVOID) &Core[cpu]);
	}
	return STATUS_SUCCESS;
}
