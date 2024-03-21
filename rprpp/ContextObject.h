#pragma once

#include <boost/uuid/uuid.hpp>

namespace rprpp
{
class Context;

class ContextObject 
{
public:
    explicit ContextObject(Context* context);

    ContextObject(const ContextObject&)           = delete;
    ContextObject& operator=(const ContextObject) = delete;

private:
    Context* m_context;
    boost::uuids::uuid m_tag;
};

}