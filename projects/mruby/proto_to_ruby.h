#include <cstdint>
#include <cstddef>
#include <string>

namespace ruby_fuzzer {
	class Function;

	std::string FunctionToString(const Function &input);
	std::string ProtoToCxx(const uint8_t *data, size_t size);
}