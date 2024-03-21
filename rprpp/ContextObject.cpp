#include "ContextObject.h"
#include "Context.h"

#include <cassert>

namespace rprpp
{

ContextObject::ContextObject(Context* context)
: m_context(context)
, m_tag(m_context->tagGenerator()())
{
    assert(m_context);
}

}