#include <mkmi.h>

#include "xhci.hpp"
#include "capability.hpp"

void InitializeXHCIDevice(Heap *kernelHeap, MemoryMapper *mapper, PCIHeader0_t *header0, PCICapability_t *pciCapabilityArray, usize pciCapabilityCount) {

	uptr barAddr = GetBAR(header0->BAR[0], header0->BAR[1]);

	Capability barCapability;
	AddressCapability(barAddr, &barCapability);
	volatile XHCICapabilityRegs_t *capability = (volatile XHCICapabilityRegs_t*)mapper->MMap(barCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

	volatile XHCIOperationalRegs_t *operational = (volatile XHCIOperationalRegs_t*)((uptr)capability + capability->Length);
	mkmi_log("Status 0x%x\r\n", operational->USBStatus);


	u8 maxPorts = capability->StructuralParameters1 >> 24;
	mkmi_log("XHCI revision 0x%x\r\n", capability->InterfaceVersion);
	mkmi_log("Max ports: %d\r\n", maxPorts);

	volatile XHCIPortRegs_t *portRegs = (volatile XHCIPortRegs_t*)((uptr)capability + 0x400);
	for (int i = 0; i < maxPorts; ++i) {
		mkmi_log("Status: 0x%x\r\n"
			"Power managment: 0x%x\r\n"
			"Link info: 0x%x\r\n",
			portRegs[i].StatusCtl,
			portRegs[i].PowerManagmentStatusCtl,
			portRegs[i].LinkInfo
			);
	}
}
