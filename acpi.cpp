#include "acpi.hpp"
#include "capability.hpp"
#include "virtio.hpp"

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

uptr GetBAR(u32 bar, u32 nextBar) {
	if (bar & 0b1) {
		uptr addr = bar & 0xFFF0;
		return addr;
	} else {
		u8 type = ((bar & 0b110) >> 1);
		if (type == 0) {
			uptr addr = bar & 0xFFFFFFF0;
			return addr;
		} else if (type == 2) {
			//64 bit bar
			uptr addr = ((bar & 0xFFFFFFF0) + (((uptr)nextBar & 0xFFFFFFFF) << 32));
			return addr;
		}
	}
}

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
			
			//mkmi_log("Bus addr: 0x%x\r\n", busAddress);

			for (usize device = 0; device < 32; ++device) {
				u64 offset = device << 15;
				uptr deviceAddress = busAddress + offset;

				Capability deviceCapability;
				AddressCapability(deviceAddress, &deviceCapability);
				PCIDeviceHeader_t *header = (PCIDeviceHeader_t*)mapper->MMap(deviceCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

				if(header->DeviceID == 0) continue;
				if(header->DeviceID == 0xFFFF) continue;
				
				//mkmi_log(" Device addr: 0x%x\r\n", deviceAddress);

				for (usize function = 0; function < 8; ++function) {
					u64 offset = function << 12;
					uptr functionAddress = deviceAddress + offset;

					Capability functionCapability;
					AddressCapability(functionAddress, &functionCapability);
					PCIDeviceHeader_t *header = (PCIDeviceHeader_t*)mapper->MMap(functionCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

					if(header->DeviceID == 0) continue;
					if(header->DeviceID == 0xFFFF) continue;
					
					//mkmi_log("  Function addr: 0x%x\r\n", functionAddress);
					//mkmi_log("   ID: %x:%x\r\n", header->DeviceID, header->VendorID);

					if (header->HeaderType == 0) {
						PCIHeader0_t *header0 = (PCIHeader0_t*)header;

						if (header0->VendorID == 0x1af4 &&
						   (header0->DeviceID >= 0x1000 && header0->DeviceID <= 0x107F)) {
							usize mainBar;
							usize cfgOffset;

							PCICapability_t *pciCapability = (PCICapability_t*)((uptr)header0 + header0->CapabilitiesPointer);
							for (;;) {
								mkmi_log("Capability:\r\n"
									 " ID: %x\r\n"
									 " Next: 0x%x\r\n"
									 " Length: 0x%x\r\n"
									 " CFG Type: 0x%x\r\n"
									 " BAR: 0x%x\r\n"
									 " Offset: 0x%x\r\n"
									 " Length: 0x%x\r\n", 
									 pciCapability->CapID,
									 pciCapability->CapNext,
									 pciCapability->CapLength,
									 pciCapability->CfgType,
									 pciCapability->BAR,
									 pciCapability->Offset,
									 pciCapability->Length
									 );

								if (pciCapability->CfgType == 1) {
									mkmi_log("Main bar is at: %x\r\n", pciCapability->BAR);
									mainBar = pciCapability->BAR;
								} else if (pciCapability->CfgType == 4) {
									mkmi_log("Cfg bar is at: %x\r\n", pciCapability->BAR);
									mkmi_log("Cfg offset is at: %x\r\n", pciCapability->Offset);
									cfgOffset = pciCapability->Offset;
								}

								if (pciCapability->CapNext == 0) break;
								
								pciCapability = (PCICapability_t*)((uptr)header0 + pciCapability->CapNext);
							}

							uptr addr;
							if (mainBar < 5) {
								addr = GetBAR(header0->BAR[mainBar], header0->BAR[mainBar + 1]);
							} else {
								addr = GetBAR(header0->BAR[mainBar], 0);
							}
							Capability barCapability;
							AddressCapability(addr, &barCapability);
								
							volatile VirtIOHeader_t *virtio = (volatile VirtIOHeader_t*)mapper->MMap(barCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

							
							virtio->DeviceStatus = DEVICE_ACK;
							virtio->DeviceFeatureSelect = virtio->DeviceFeatures;
							virtio->DeviceStatus = DEVICE_ACK | DRIVER_LOAD;

							mkmi_log("Device has %d queues\r\n", virtio->NumQueues);
							for (usize i = 0; i < virtio->NumQueues; ++i) {
								virtio->QueueSelect = i;

								u32 queueSize = virtio->QueueSize;
								u32 sizeofBuffers = (sizeof(VirtIOQueueBuffer_t) * queueSize);
								u32 sizeofQueueAvailable = (2*sizeof(u16)) + (queueSize*sizeof(u16)); 
								u32 sizeofQueueUsed = (2*sizeof(u16))+(queueSize*sizeof(VirtIOUsedItem_t));
								u32 sizeTotal = sizeofBuffers + sizeofQueueAvailable;
								ROUND_UP_TO(sizeofQueueUsed, PAGE_SIZE);
								sizeTotal += sizeofQueueUsed;
								ROUND_UP_TO(sizeTotal, PAGE_SIZE);

								Capability utMemory;
								GetUntypedRegion(sizeTotal, &utMemory);
								usize pageCount = sizeTotal / PAGE_SIZE;
								Capability mmioMemory[pageCount];
								RetypeCapability(utMemory, mmioMemory, MMIO_MEMORY, pageCount);

								mkmi_log("Memory region for queue: 0x%x %d\r\n", mmioMemory[0].Object, sizeTotal);

								uptr addr = mmioMemory[0].Object;
								virtio->QueueDesc = addr;
								virtio->QueueAvail = addr + sizeofBuffers;
								virtio->QueueUsed = ((addr + sizeofBuffers + sizeofQueueAvailable +0xFFF)&~0xFFF);

								u64 desc = virtio->QueueDesc;
								mkmi_log("Desc: 0x%x\r\n", desc);
								u64 avail = virtio->QueueAvail;
								mkmi_log("Avail: 0x%x\r\n", avail);
								u64 used = virtio->QueueUsed;
								mkmi_log("Used: 0x%x\r\n", used);

								virtio->QueueEnable = 1;
							}


							switch(header0->SubsystemID) {
								case 0x1:
								case 0x41: {
									mkmi_log("VirtIO Network Adapter\r\n");
									virtio->DeviceStatus = DEVICE_ACK | DRIVER_LOAD | DRIVER_READY;
									
									VirtIONetHeader_t *network = (VirtIONetHeader_t*)((uptr)virtio + cfgOffset);
									mkmi_log("MAC Address: %x-%x-%x-%x-%x-%x\r\n",
										network->MacAddress[0],
										network->MacAddress[1],
										network->MacAddress[2],
										network->MacAddress[3],
										network->MacAddress[4],
										network->MacAddress[5]
										);
									}
									break;
								case 0x2:
								case 0x42: {
									virtio->DeviceStatus = DEVICE_ACK | DRIVER_LOAD | DRIVER_READY;
									mkmi_log("VirtIO Block Device\r\n");

									VirtIOBlockHeader_t *block = (VirtIOBlockHeader_t*)((uptr)virtio + cfgOffset);
									mkmi_log("Capacity: %d (%dMB)\r\n", block->Capacity, block->Capacity * 512 / 1024 / 1024);
									/*
									Capability descCapability;
									virtio->QueueSelect = 0;
									AddressCapability(virtio->QueueDesc, &descCapability);
									void *desc = mapper->MMap(descCapability,
										PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

									VirtIOQueueBuffer_t *buf = (VirtIOQueueBuffer_t*)desc;
									buf->addr = 0;
									desc->len = 0;
									desc->flags*/
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
									mkmi_log("VirtIO IO RPMSG\r\n");
									break;
								case 0x8:
								case 0x48:
									mkmi_log("VirtIO IO SCSI\r\n");
									break;
								case 0x9:
								case 0x49:
									mkmi_log("VirtIO IO 9P\r\n");
									break;
								case 0xA:
								case 0x4A:
									mkmi_log("VirtIO IO WLAN\r\n");
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

