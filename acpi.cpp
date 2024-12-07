#include "acpi.hpp"
#include "capability.hpp"

#include <mkmi.h>

int InitMCFG(MemoryMapper *mapper, MCFG_t *mcfg);

void InitACPI(MemoryMapper *mapper, ContainerInfo *info) {
	Capability rsdpCapability;
	AddressCapability(info->x86_64.RSDPCapability, &rsdpCapability);

	void *rsdpAddr = mapper->MMap(rsdpCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	RSDP_t *rsdp = (RSDP_t*)((uptr)rsdpAddr + info->x86_64.RSDPOffset);
	mkmi_log("rsdp at 0x%x\r\n", rsdp);

	{
		char signature[9] = { '\0' };
		for (int i = 0; i < 8; ++i) {
			signature[i] = rsdp->Signature[i];
		}
	
		mkmi_log("Signature: %s\r\n", signature);
	}

	mkmi_log("RSDT: 0x%x\r\n", rsdp->RsdtAddress);
	mkmi_log("XSDT: 0x%x\r\n", rsdp->XsdtAddress);

	uptr sdtAddr = rsdp->XsdtAddress;
	ROUND_DOWN_TO(sdtAddr, PAGE_SIZE);

	Capability xsdtCapability;
	AddressCapability(sdtAddr, &xsdtCapability);
	void *xsdtAddr = mapper->MMap(xsdtCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	SDTHeader_t *xsdt = (SDTHeader_t*)((uptr)xsdtAddr + (rsdp->XsdtAddress % PAGE_SIZE));

	{
		char signature[5] = { '\0' };
		for (int i = 0; i < 4; ++i) {
			signature[i] = xsdt->Signature[i];
		}
		
		mkmi_log("Signature: %s\r\n", signature);
	}

	int entries = (xsdt->Length - sizeof(SDTHeader_t)) / 8;

	for (int i = 0; i < entries; i++) {
		char sig[5] = { '\0' };
		uptr *ptr = (uptr*)((uptr)xsdt + sizeof(SDTHeader_t) + i * 8);

		mkmi_log(" -> 0x%x\r\n", ptr);

		Capability sdtCapability;
		AddressCapability(*ptr, &sdtCapability);
		void *sdtAddr = mapper->MMap(sdtCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
		SDTHeader_t *sdt = (SDTHeader_t*)((uptr)sdtAddr);

		memcpy(sig, sdt->Signature, 4);
		mkmi_log("%d: 0x%x -> %s\r\n", i, *ptr, sig);
		if (memcmp(sdt->Signature, "MCFG", 4) == 0) {
			MCFG_t *mcfg = (MCFG_t*)sdt;
			InitMCFG(mapper, mcfg);
		}

	}
}
/*
inline static void CheckBar(u32 bar, u32 nextBar) {
	KInfo *info = GetInfo();

	if (bar & 0b1) {
		// IO SPACE BAR
		// TODO: manage io ports
		uptr addr = bar & 0xFFF0;
		if (addr != 0) {
			mkmi_log("    16 bit BAR at 0x%x\r\n", addr);
		}
	} else {
		u8 type = (bar & 0b110 >> 1);
		if (type == 0) {
			// 32 bit bar
			uptr addr = bar & 0xFFFFFFF0;
			if (addr != 0) {
				mkmi_log("    32 bit BAR at 0x%x\r\n", addr);
				PMM::CheckSpace(info->RootCSpace, DEFAULT_CHECK_SPACE);
				CAPABILITY::GenerateCapability(info->RootCSpace, MMIO_MEMORY, addr, ACCESS | READ | WRITE);
			}
		} else if (type == 2) {
			//64 bit bar
			uptr addr = ((bar & 0xFFFFFFF0) + (((uptr)nextBar & 0xFFFFFFFF) << 32));
			if (addr != 0) {
				mkmi_log("    64 bit BAR at 0x%x\r\n", addr);
				PMM::CheckSpace(info->RootCSpace, DEFAULT_CHECK_SPACE);
				CAPABILITY::GenerateCapability(info->RootCSpace, MMIO_MEMORY, addr, ACCESS | READ | WRITE);
			}
		}
	}
}*/

int InitMCFG(MemoryMapper *mapper, MCFG_t *mcfg) {
	uptr currentPtr = (uptr)&mcfg->FirstEntry;
	uptr entriesEnd = (uptr)mcfg + mcfg->Length;

	while(currentPtr < entriesEnd) {
		MCFGEntry_t *current = (MCFGEntry_t*)currentPtr;
		mkmi_log("Device addr: 0x%x\r\n"
				            "Seg:         0x%x\r\n"
					    "Start bus:   0x%x\r\n"
					    "End bus:     0x%x\r\n", 
					    current->BaseAddress, current->PCISeg,
					    current->StartPCIBus, current->EndPCIBus);


		for (usize bus = current->StartPCIBus; bus < current->EndPCIBus; ++bus) {
			u64 offset = bus << 20;
			uptr busAddress = current->BaseAddress + offset;

			Capability busCapability;
			AddressCapability(busAddress, &busCapability);
			PCIDeviceHeader_t *header = (PCIDeviceHeader_t*)mapper->MMap(busCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
			if(header->DeviceID == 0) continue;
			if(header->DeviceID == 0xFFFF) continue;
			
			mkmi_log("Bus addr: 0x%x\r\n", busAddress);

			for (usize device = 0; device < 32; ++device) {
				u64 offset = device << 15;
				uptr deviceAddress = busAddress + offset;

				Capability deviceCapability;
				AddressCapability(deviceAddress, &deviceCapability);
				PCIDeviceHeader_t *header = (PCIDeviceHeader_t*)mapper->MMap(deviceCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

				if(header->DeviceID == 0) continue;
				if(header->DeviceID == 0xFFFF) continue;
				
				mkmi_log(" Device addr: 0x%x\r\n", deviceAddress);

				for (usize function = 0; function < 8; ++function) {
					u64 offset = function << 12;
					uptr functionAddress = deviceAddress + offset;

					Capability functionCapability;
					AddressCapability(functionAddress, &functionCapability);
					PCIDeviceHeader_t *header = (PCIDeviceHeader_t*)mapper->MMap(functionCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

					if(header->DeviceID == 0) continue;
					if(header->DeviceID == 0xFFFF) continue;
					
					mkmi_log("  Function addr: 0x%x\r\n", functionAddress);
					mkmi_log("   ID: %x:%x\r\n", header->DeviceID, header->VendorID);

					if (header->HeaderType == 0) {
						PCIHeader0_t *header0 = (PCIHeader0_t*)header;
						mkmi_log("   Type 0\r\n"
								            "    BAR0: 0x%x\r\n"
								            "    BAR1: 0x%x\r\n"
								            "    BAR2: 0x%x\r\n"
								            "    BAR3: 0x%x\r\n"
								            "    BAR4: 0x%x\r\n"
								            "    BAR5: 0x%x\r\n",
									    header0->BAR0,
									    header0->BAR1,
									    header0->BAR2,
									    header0->BAR3,
									    header0->BAR4,
									    header0->BAR5);
					} else if (header->HeaderType == 1) {
						PCIHeader1_t *header1 = (PCIHeader1_t*)header;
						mkmi_log("   Type 1\r\n"
								            "    BAR0: 0x%x\r\n"
								            "    BAR1: 0x%x\r\n",
									    header1->BAR0,
									    header1->BAR1);

					} else if (header->HeaderType == 2) {
						PCIHeader2_t *header2 = (PCIHeader2_t*)header;
						mkmi_log("   Type 2\r\n"
								            "    Base: 0x%x\r\n",
									    header2->CardBusBaseAddress);
					} else {
						mkmi_log("   Unknown header type: %d\r\n",
								header->HeaderType);
					}
				}
			}
		}

		currentPtr += sizeof(MCFGEntry_t);
	}



	return 0;
}

