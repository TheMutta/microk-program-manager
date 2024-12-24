#pragma once
#include <cdefs.h>

#include "memory.hpp"
#include "capability.hpp"

#define XAPIC_MAX_REGISTER                                0x3E
#define X2APIC_MAX_REGISTER                               0x3F

#define APIC_REGISTER_LAPIC_ID                            0x02
#define APIC_REGISTER_LAPIC_VERSION                       0x03
#define APIC_REGISTER_THREAD_PRIORITY                       0x08
#define APIC_REGISTER_ARBITRATION_PRIORITY                0x09
#define APIC_REGISTER_PROCESSOR_PRIORITY                  0x0A
#define APIC_REGISTER_EOI                                 0x0B
#define APIC_REGISTER_REMOTE_READ                         0x0C
#define APIC_REGISTER_LOGICAL_DESTINATION                 0x0D
#define APIC_REGISTER_DESTINATION_FORMAT                  0x0E
#define APIC_REGISTER_SPURIOUS_INTERRUPT_VECTOR           0x0F
#define APIC_REGISTER_IN_SERVICE(n)                      (0x10 + n)
#define APIC_REGISTER_IN_SERVICE_MAX_COUNT                8
#define APIC_REGISTER_TRIGGER_MODE(n)                    (0x18 + n)
#define APIC_REGISTER_TRIGGER_MODE_MAX_COUNT              8
#define APIC_REGISTER_INTERRUPT_REQUEST(n)               (0x20 + n)
#define APIC_REGISTER_INTERRUOT_REQUEST_MAX_COUNT         8
#define APIC_REGISTER_ERROR_STATUS                        0x28
#define APIC_REGISTER_INTERRUPT_COMMAND_REGISTER          0x30 /* In x2APIC mode, all the MSR is used for 64 bits */
#define APIC_REGISTER_LVT_TIMER_REGISTER                  0x32
#define APIC_REGISTER_LVT_TERMAL_SENSOR                   0x33
#define APIC_REGISTER_LVT_PERFORMANCE_MONITORING_COUNTERS 0x34
#define APIC_REGISTER_LVT_LINT0                           0x35
#define APIC_REGISTER_LVT_LINT1                           0x36
#define APIC_REGISTER_LVT_ERROR                           0x37
#define APIC_REGISTER_TIMER_INITIAL_COUNT                 0x38
#define APIC_REGISTER_TIMER_CURRENT_COUNT                 0x39
#define APIC_REGISTER_TIMER_DIVIDE_CONFIGURATION          0x3E
#define APIC_REGISTER_SELF_IPI                            0x3F

#define APIC_REGISTER_IN_SERVICE_MAX_COUNT                8
#define APIC_REGISTER_TRIGGER_MODE_MAX_COUNT              8
#define APIC_REGISTER_INTERRUOT_REQUEST_MAX_COUNT         8
#define APIC_REGISTER_INTERRUPT_COMMAND_REGISTER_MAX      2


#define APIC_SPURIOUS_ACTIVATE     (1 << 8)
#define APIC_LVT_TIMER_MASK        0x10000
#define APIC_LVT_TIMER_DIVIDE_1    0x3
#define APIC_LVT_TIMER_ONESHOT     (0 << 17)
#define APIC_LVT_TIMER_PERIODIC    (1 << 17)
#define APIC_LVT_TIMER_TSCDEADLINE (2 << 17)

#define MSR_APIC_BASE    0x0000001B

#define MSR_TSC_DEADLINE 0x000006E0

#define MSR_x2APIC_BEGIN 0x00000800
#define MSR_x2APIC_END   0x000008FF

struct APIC {
	u32 ID;

	Capability APICCapability;

	uptr Base;
	uptr MappedAddress;

	bool ProcessorIsBSP;
	bool x2APICMode;
};

APIC *InitializeAPIC(MemoryMapper *mapper, Heap *kernelHeap);
void ArmTimer(APIC *apic, usize quantity);
