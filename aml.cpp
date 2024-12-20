#include "aml.hpp"
#include "acpi.hpp"
#include "aml_opcodes.hpp"

#include <mkmi.h>

void ParseAML(SDTHeader_t *table, Heap *kernelHeap) {
	u8 *opcode = (u8*)((uptr)table + sizeof(SDTHeader_t));

	for (usize i = 0; i < table->Length - sizeof(SDTHeader_t); ++i) {
		switch(opcode[i]) {
			case SCOPE_OP: {
				mkmi_log("Scope\r\n");
				}
				break;
			case EXTOP_PREFIX: {
				switch(opcode[++i]) {
					default:
						mkmi_log("Extended Opcode: 0x%x\r\n", opcode[i]);
				}
				}
			default:
				mkmi_log("Opcode: 0x%x\r\n", opcode[i]);
		}


	}
}
