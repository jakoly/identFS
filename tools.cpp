#include "tools.h"

tools::tools() {}

std::string guidToString(const GUID &guid) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(8) << guid.Data1 << "-"
       << std::setw(4) << guid.Data2 << "-"
       << std::setw(4) << guid.Data3 << "-";
    for (int i = 0; i < 2; ++i)
        ss << std::setw(2) << static_cast<int>(guid.Data4[i]);
    ss << "-";
    for (int i = 2; i < 8; ++i)
        ss << std::setw(2) << static_cast<int>(guid.Data4[i]);
    return ss.str();
}
