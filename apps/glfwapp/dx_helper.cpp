#include "dx_helper.h"
#include <sstream>

#include <comdef.h>

void process_dx_error(HRESULT res, const char* file, int line)
{
   std::stringstream stream;

    _com_error err(res);

    stream << "DX HRESULT is " << res << " in " << file << " at line " << line << "\n";
    stream << err.ErrorMessage() << "\n";

    throw std::runtime_error(stream.str());
}