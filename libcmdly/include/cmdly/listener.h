/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_LISTENER_H
#define CMDLY_LISTENER_H

#include <string>
#include <forward_list>
#include <cmdly/line.h>
#include <cmdly/cursor.h>

namespace cmdly {

class Terminal;

template<typename... Args>
class Listener
{
public:
    enum class Status
    {
        OK, CONTINUE, BREAK
    };

    virtual ~Listener() = default;
    virtual Status invoke(Args... args) = 0;
    using FunctionType = std::function<Status(Args...args)>;
}; /* End of class Listener */

template<typename... Args>
class ListenerFunctionWrapper : public Listener<Args...>
{
public:
    explicit ListenerFunctionWrapper(typename Listener<Args...>::FunctionType handler) :
        handler_(std::move(handler))
    {}

    typename Listener<Args...>::Status invoke(Args...args) override
    {
        return handler_(std::forward<Args>(args)...);
    }

private:
    typename Listener<Args...>::FunctionType handler_;
}; /* End of class ListenerFunctionWrapper */

using KeyPressedListener = Listener<const Key &, Line &, Cursor &, Terminal &>;
using KeyPressedListenerFunctionWrapper = ListenerFunctionWrapper<const Key &, Line &, Cursor &, Terminal &>;
using LineChangedListener = Listener<const std::string &, Line &, Cursor &, Terminal &>;
using LineChangedListenerFunctionWrapper = ListenerFunctionWrapper<const std::string &, Line &, Cursor &, Terminal &>;
using LineEnteredListener = Listener<const std::string &, Terminal &>;
using LineEnteredListenerFunctionWrapper = ListenerFunctionWrapper<const std::string &, Terminal &>;

} /* End of namespace cmdly */

#endif /* !CMDLY_LISTENER_H */
