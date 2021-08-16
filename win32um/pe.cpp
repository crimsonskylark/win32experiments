#include "Introspect.hh"

int introspect() {
	auto h = GetModuleHandle(NULL);
	if (!h) {
		std::cout << "unable to get module handle"
			<< "\n";
		return -1;
	}

	auto dosHdr = reinterpret_cast<IMAGE_DOS_HEADER*>(h);
	auto ntHdr = reinterpret_cast<IMAGE_NT_HEADERS*>(
		reinterpret_cast<uint8_t*>(dosHdr) + dosHdr->e_lfanew);

	auto sctHdr = reinterpret_cast<IMAGE_SECTION_HEADER*>(ntHdr + 1);
	std::cout << "ntHdr ptr: " << std::hex << ntHdr << "\n";
	std::cout << "ntHdr+1 ptr: " << std::hex << ntHdr + 1 << "\n";

	for (auto idx = 0; idx < ntHdr->FileHeader.NumberOfSections; idx++) {
		std::cout << sctHdr[idx].Name << "\n";
	}

	std::cout << dosHdr->e_lfanew << "\n";

	return 1;
}