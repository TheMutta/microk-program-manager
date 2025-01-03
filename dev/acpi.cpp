#include "../init/capability.hpp"
#include "acpi.hpp"
#include "virtio.hpp"
#include "virtio-disk.hpp"
#include "virtio-gpu.hpp"
#include "virtio-net.hpp"
#include "xhci.hpp"
#include "e1000.hpp"
#include "../net/arp.hpp"

#include <mkmi.h>

int InitMCFG(Heap *kernelHeap, MemoryMapper *mapper, MCFG_t *mcfg);
int InitFADT(Heap *kernelHeap, MemoryMapper *mapper, FADT_t *fadt);

void InitACPI(Heap *kernelHeap, MemoryMapper *mapper, ContainerInfo *info) {
	Capability rsdpCapability;
	AddressCapability(info->x86_64.RSDPCapability, &rsdpCapability);

	void *rsdpAddr = mapper->MMap(&rsdpCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
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
	void *xsdtAddr = mapper->MMap(&xsdtCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
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
		void *sdtAddr = mapper->MMap(&sdtCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
		SDTHeader_t *sdt = (SDTHeader_t*)((uptr)sdtAddr);

		memcpy(sig, sdt->Signature, 4);
		mkmi_log("%d: 0x%x -> %s\r\n", i, *ptr, sig);
		if (memcmp(sdt->Signature, "MCFG", 4) == 0) {
			MCFG_t *mcfg = (MCFG_t*)sdt;
			InitMCFG(kernelHeap, mapper, mcfg);
		} else if (memcmp(sdt->Signature, "FACP", 4) == 0) {
			FADT_t *fadt = (FADT_t*)sdt;
			InitFADT(kernelHeap, mapper, fadt);
		}

	}
}

int InitFADT(Heap *kernelHeap, MemoryMapper *mapper, FADT_t *fadt) {
	SDTHeader_t *dsdt;
	Capability dsdtCapability;
	if (fadt->X_Dsdt) {
		AddressCapability(fadt->X_Dsdt, &dsdtCapability);
	} else if (fadt->Dsdt) {
		AddressCapability(fadt->Dsdt, &dsdtCapability);
	}

	dsdt = (SDTHeader_t*)mapper->MMap(&dsdtCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	usize length = dsdt->Length;
	mapper->MUnmap(&dsdtCapability, 1);

	usize count;
	Capability dsdtCapabilities[length / PAGE_SIZE];
	for (int i = 0; i < count; ++i ) {
		AddressCapability((uptr)dsdt + i * PAGE_SIZE, &dsdtCapabilities[i]);
	}

	dsdt = (SDTHeader_t*)mapper->MMap(&dsdtCapability, count, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

	//ParseAML(dsdt, kernelHeap);

	return 0;
}

uptr GetBAR(u32 *bar, u32 *nextBar, usize *size) {
	if (*bar & 0b1) {
		// IO SPACE BAR
		// TODO: manage io ports
		uptr addr = *bar & 0xFFF0;
		usize barSize = 1;

		volatile u16 originalBar = *bar;
		*bar = 0xffff;
		barSize = (~(*bar) + 1) & 0xffff;
		*bar = originalBar;

		if (addr != 0) {
			*size = barSize;
		}
	} else {
		u8 type = ((*bar & 0b110) >> 1);
		if (type == 0) {
			// 32 bit bar
			uptr addr = *bar & 0xfffffff0;
			usize barSize = PAGE_SIZE;

			volatile u32 originalBar = *bar;
			*bar = 0xffffffff;
			barSize = ~(*bar) + 1;
			ROUND_UP_TO(barSize, PAGE_SIZE);
			*bar = originalBar;

			if (addr != 0) {
				*size = barSize;
				return addr;
			}
		} else if (type == 2) {
			//64 bit bar
			uptr addr = ((*bar & 0xFFFFFFF0) + (((uptr)*nextBar & 0xFFFFFFFF) << 32));
			usize barSize = PAGE_SIZE;
			
			volatile u32 originalBar = *bar;
			*bar = 0xFFFFFFFF;
			barSize = ~(*bar) + 1;
			ROUND_UP_TO(barSize, PAGE_SIZE);
			*bar = originalBar;

			if (addr != 0) {
				*size = barSize;
				return addr;
			}
		}
	}
}

int InitMCFG(Heap *kernelHeap, MemoryMapper *mapper, MCFG_t *mcfg) {
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
			PCIDeviceHeader_t *header = (PCIDeviceHeader_t*)mapper->MMap(&busCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
			if(header->DeviceID == 0) continue;
			if(header->DeviceID == 0xFFFF) continue;
			
			//mkmi_log("Bus addr: 0x%x\r\n", busAddress);

			for (usize device = 0; device < 32; ++device) {
				u64 offset = device << 15;
				uptr deviceAddress = busAddress + offset;

				Capability deviceCapability;
				AddressCapability(deviceAddress, &deviceCapability);
				PCIDeviceHeader_t *header = (PCIDeviceHeader_t*)mapper->MMap(&deviceCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

				if(header->DeviceID == 0) continue;
				if(header->DeviceID == 0xFFFF) continue;
				
				//mkmi_log(" Device addr: 0x%x\r\n", deviceAddress);

				for (usize function = 0; function < 8; ++function) {
					u64 offset = function << 12;
					uptr functionAddress = deviceAddress + offset;

					Capability functionCapability;
					AddressCapability(functionAddress, &functionCapability);
					PCIDeviceHeader_t *header = (PCIDeviceHeader_t*)mapper->MMap(&functionCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

					if(header->DeviceID == 0) continue;
					if(header->DeviceID == 0xFFFF) continue;
					
					//mkmi_log("  Function addr: 0x%x\r\n", functionAddress);
					//mkmi_log("   ID: %x:%x\r\n", header->DeviceID, header->VendorID);

					if (header->HeaderType == 0) {
						PCIHeader0_t *header0 = (PCIHeader0_t*)header;
						// XHCI
						if (header0->Class == 0x0C && header0->Subclass == 0x03 && header0->ProgIF == 0x30) {
							InitializeXHCIDevice(kernelHeap, mapper, header0);
							/*

							AddressCapability(barAddr, &device->BARCapability);
							 = (volatile *)mapper->MMap(device->BARCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);*/
						}

						if (header0->VendorID == 0x8086 && header0->DeviceID == 0x100E) {
							mkmi_log("E1000 found.\r\n");
							E1000_t *e1000 = InitializeE1000(kernelHeap, mapper, header0);
/*
							u8 ip[ARP_PACKET_IPv4_LENGTH] = { 192, 168, 1, 110 };
							u8 destArp[ARP_PACKET_ETH_LENGTH] = {0,0,0,0,0,0};
							ARPSendPacket(e1000, kernelHeap, ip, destArp);*/
						}

						/* Virtio Devices */
						if (header0->VendorID == 0x1af4 &&
						   (header0->DeviceID >= 0x1000 && header0->DeviceID <= 0x107F)) {
							VirtIODevice_t *device = InitializeVirtIODevice(kernelHeap, mapper, header0);

							if (header0->DeviceID == 0x1050) {
								mkmi_log("VirtIO GPU\r\n");
								VirtIOGPU_t *gpu = InitializeVirtIOGPU(kernelHeap, mapper, device);

							}


							switch(header0->SubsystemID) {
								case 0x1:
								case 0x41: {
									mkmi_log("VirtIO Network Adapter\r\n");
									
									VirtIONetDevice_t *netDevice = InitializeVirtIONetDevice(kernelHeap, mapper, device);
									}
									break;
								case 0x2:
								case 0x42: {
									mkmi_log("VirtIO Block Device\r\n");

									VirtIOBlockDevice_t *blkDevice = InitializeVirtIOBlockDevice(kernelHeap, mapper, device);

									}
									break;
								case 0x3:
								case 0x43:
									mkmi_log("VirtIO Console\r\n");
									break;
								case 0x4:
								case 0x44:
									mkmi_log("VirtIO RNG\r\n");
									break;
								case 0x5:
								case 0x45:
									mkmi_log("VirtIO Memory Ballooning\r\n");
									break;
								case 0x6:
								case 0x46:
									mkmi_log("VirtIO IO Memory\r\n");
									break;
								case 0x7:
								case 0x47:
									mkmi_log("VirtIO RPMSG\r\n");
									break;
								case 0x8:
								case 0x48:
									mkmi_log("VirtIO SCSI\r\n");
									break;
								case 0x9:
								case 0x49:
									mkmi_log("VirtIO 9P\r\n");
									break;
								}



						}
						/*mkmi_log("   Type 0\r\n"
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
									    header0->BAR5);*/
					} else if (header->HeaderType == 1) {
						PCIHeader1_t *header1 = (PCIHeader1_t*)header;
						/*
						mkmi_log("   Type 1\r\n"
								            "    BAR0: 0x%x\r\n"
								            "    BAR1: 0x%x\r\n",
									    header1->BAR0,
									    header1->BAR1);*/
					} else if (header->HeaderType == 2) {
						/*
						PCIHeader2_t *header2 = (PCIHeader2_t*)header;
						mkmi_log("   Type 2\r\n"
								            "    Base: 0x%x\r\n",
									    header2->CardBusBaseAddress);*/
					} else {
						/*mkmi_log("   Unknown header type: %d\r\n",
								header->HeaderType);*/
					}
				}
			}
		}

		currentPtr += sizeof(MCFGEntry_t);
	}



	return 0;
}

