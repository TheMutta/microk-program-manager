#include "e1000.hpp"

#include <mkmi.h>

#include "../init/capability.hpp"

static void WriteCommand(E1000_t *device, u16 address, u32 value) {
	*(volatile u32*)((uptr)device->BARMapping + address) = value;
}

static u32 ReadCommand(E1000_t *device, u16 address) {
	return *(volatile u32*)((uptr)device->BARMapping + address);
}

static u32  EEPROMRead(E1000_t *device, uint8_t addr) {
	u16 data = 0;
	u32 tmp = 0;

	WriteCommand(device, REG_EEPROM, (1) | ((u32)(addr) << 8) );
	while( !((tmp = ReadCommand(device, REG_EEPROM)) & (1 << 4)) );

	data = (u16)((tmp >> 16) & 0xFFFF);
	return data;
}

static void RXTXInit(E1000_t *device, MemoryMapper *mapper) {
	uptr ptr;

	usize sizeRX = sizeof(E1000RXDesc_t)*E1000_NUM_RX_DESC + 16;
	usize sizeTX = sizeof(E1000TXDesc_t)*E1000_NUM_TX_DESC + 16;

	mkmi_log("E1000 RX buffer size: %d\r\n", sizeRX);
	mkmi_log("E1000 TX buffer size: %d\r\n", sizeTX);

	Capability utBufferCapability;

	Capability rxBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &rxBufferCapability, FRAME_MEMORY, 1);

	Capability txBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &txBufferCapability, FRAME_MEMORY, 1);

	device->RXDescs = rxBufferCapability.Object;
	device->TXDescs = txBufferCapability.Object;
	device->RXDescsMapping = (E1000RXDesc_t*)mapper->MMap(&rxBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	device->TXDescsMapping = (E1000TXDesc_t*)mapper->MMap(&txBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(device->RXDescsMapping, 0, PAGE_SIZE);
	memset(device->TXDescsMapping, 0, PAGE_SIZE);
	mkmi_log("RX: 0x%x TX: 0x%x\r\n", device->RXDescsMapping, device->TXDescsMapping);


	for(int i = 0; i < E1000_NUM_RX_DESC; i++) {
		usize rxBufSize = 8192 + 16;
		ROUND_UP_TO(rxBufSize, PAGE_SIZE);

		Capability rxBufferCapability[rxBufSize / PAGE_SIZE];
		ASSERT(GetUntypedRegion(rxBufSize, &utBufferCapability) == GUNTPD_OK);
		RetypeCapability(utBufferCapability, rxBufferCapability, FRAME_MEMORY, rxBufSize / PAGE_SIZE);
		device->RXDescsMapping[i].Addr = rxBufferCapability[0].Object;
		device->RXDescsMapping[i].Status = 0;

		mkmi_log("OK\r\n");
	}

	for(int i = 0; i < E1000_NUM_TX_DESC; i++) {
		device->TXDescsMapping[i].Addr = 0;
		device->TXDescsMapping[i].Cmd = 0;
		device->TXDescsMapping[i].Status = TSTA_DD;
	}

	WriteCommand(device, REG_RXDESCLO, (uint32_t)(uint64_t)rxBufferCapability.Object >> 32);
	WriteCommand(device, REG_RXDESCHI, (uint32_t)((uint64_t)rxBufferCapability.Object & 0xFFFFFFFF));

	WriteCommand(device, REG_TXDESCLO, (uint32_t)((uint64_t)txBufferCapability.Object >> 32));
	WriteCommand(device, REG_TXDESCHI, (uint32_t)((uint64_t)txBufferCapability.Object & 0xFFFFFFFF));

	WriteCommand(device, REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);
	WriteCommand(device, REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);

	WriteCommand(device, REG_RXDESCHEAD, 0);
	WriteCommand(device, REG_RXDESCTAIL, E1000_NUM_RX_DESC-1);
	WriteCommand(device, REG_TXDESCHEAD, 0);
	WriteCommand(device, REG_TXDESCTAIL, 0);

	device->RXCurr = 0;
	device->TXCurr = 0;
	WriteCommand(device, REG_RCTRL, RCTL_EN
			| RCTL_SBP
			| RCTL_UPE
			| RCTL_MPE
			| RCTL_LBM_NONE
			| RTCL_RDMTS_HALF
			| RCTL_BAM
			| RCTL_SECRC
			| RCTL_BSIZE_8192);

	WriteCommand(device, REG_TCTRL,  TCTL_EN
			| TCTL_PSP
			| (15 << TCTL_CT_SHIFT)
			| (64 << TCTL_COLD_SHIFT)
			| TCTL_RTLC);


	// This line of code overrides the one before it but I left both to highlight that the previous one works with e1000 cards, but for the e1000e cards 
	// you should set the TCTRL register as follows. For detailed description of each bit, please refer to the Intel Manual.
	// In the case of I217 and 82577LM packets will not be sent if the TCTRL is not configured using the following bits.
	WriteCommand(device, REG_TCTRL,  0b0110000000000111111000011111010);
	WriteCommand(device, REG_TIPG,  0x0060200A);
}

static void EnableInterrupt(E1000_t *device) {
	WriteCommand(device, REG_IMASK ,0x1F6DC);
	WriteCommand(device, REG_IMASK ,0xff & ~4);
	ReadCommand(device, 0xc0);
}

E1000_t *InitializeE1000(Heap *kernelHeap, MemoryMapper *mapper, PCIHeader0_t *header0, PCICapability_t *pciCapabilityArray, usize pciCapabilityCount) {
	E1000_t *device = (E1000_t*)kernelHeap->Malloc(sizeof(E1000_t));

	/* Enable busmaster */
	header0->Command |= 0b100;


	mkmi_log("E1000 has %d capabilities.\r\n", pciCapabilityCount);

	for (usize i = 0; i < pciCapabilityCount; ++i) {
		mkmi_log("Capability:\r\n"
		  " ID: %x\r\n"
		  " Next: 0x%x\r\n"
		  " Length: 0x%x\r\n"
		  " CFG Type: 0x%x\r\n"
		  " BAR: 0x%x\r\n"
		  " Offset: 0x%x\r\n"
		  " Length: 0x%x\r\n", 
		  pciCapabilityArray[i].CapID,
		  pciCapabilityArray[i].CapNext,
		  pciCapabilityArray[i].CapLength,
		  pciCapabilityArray[i].CfgType,
		  pciCapabilityArray[i].BAR,
		  pciCapabilityArray[i].Offset,
		  pciCapabilityArray[i].Length
		  );
	}
	
	usize size;
	uptr barAddr = GetBAR(&header0->BAR[0], &header0->BAR[1], &size);

	usize count = size / PAGE_SIZE;
	device->BARCapability = (Capability*)kernelHeap->Malloc(sizeof(Capability)*count);

	for (usize i = 0; i < count; ++i) {
		AddressCapability(barAddr + i * PAGE_SIZE, &device->BARCapability[i]);
	}

	device->BARMapping = (volatile u8*)mapper->MMap(device->BARCapability, count, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);


	WriteCommand(device, REG_EEPROM, 0x1);
	u32 val = ReadCommand(device, REG_EEPROM);
	if (val & 0x10) {
		mkmi_log("E1000 EEPROM exists.\r\n");

		u32 temp;
		temp = EEPROMRead(device, 0);
		device->MAC[0] = temp &0xff;
		device->MAC[1] = temp >> 8;
		temp = EEPROMRead(device, 1);
		device->MAC[2] = temp &0xff;
		device->MAC[3] = temp >> 8;
		temp = EEPROMRead(device, 2);
		device->MAC[4] = temp &0xff;
		device->MAC[5] = temp >> 8;
		

		mkmi_log("E1000 MAC: %x-%x-%x-%x-%x-%x.\r\n",
				device->MAC[0],
				device->MAC[1],
				device->MAC[2],
				device->MAC[3],
				device->MAC[4],
				device->MAC[5]);
	} else {
		mkmi_log("E1000 EEPROM doesn't exist.\r\n");
	}

	for(int i = 0; i < 0x80; i++) {
	        WriteCommand(device, 0x5200 + i*4, 0);
	}

	EnableInterrupt(device);
	RXTXInit(device, mapper);

	Capability utBufferCapability;
	Capability packetBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &packetBufferCapability, FRAME_MEMORY, 1);
	device->PacketBufferMapping = (u8*)mapper->MMap(&packetBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

	mkmi_log("E1000 started.\r\n");

	//E1000SendPacket(device, device->PacketBuffer, 32);

	return device;
}

int E1000SendPacket(E1000_t *device, uptr data, u16 len) {
	device->TXDescsMapping[device->TXCurr].Addr = data;
	device->TXDescsMapping[device->TXCurr].Length = len;
	device->TXDescsMapping[device->TXCurr].Cmd = CMD_EOP | CMD_IFCS | CMD_RS;
	device->TXDescsMapping[device->TXCurr].Status = 0;

	u16 oldCurr = device->TXCurr;
	device->TXCurr = (device->TXCurr + 1) % E1000_NUM_TX_DESC;

	WriteCommand(device, REG_TXDESCTAIL, device->TXCurr);
	while(!(device->TXDescsMapping[oldCurr].Status & 0xff));    
	return 0;
}
