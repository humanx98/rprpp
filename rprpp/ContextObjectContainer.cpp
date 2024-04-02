#include "ContextObjectContainer.h"

namespace rprpp {

boost::uuids::uuid ContextObjectContainer::generateNextTag()
{
    return m_tagGenerator();
}

} // namespace rprpp