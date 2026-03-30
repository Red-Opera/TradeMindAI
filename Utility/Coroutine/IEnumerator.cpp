#include "IEnumerator.h"

IEnumerator::promise_type::~promise_type()
{
    delete currentWait;
    currentWait = nullptr;
}

IEnumerator IEnumerator::promise_type::get_return_object()
{
    return IEnumerator{ std::coroutine_handle<promise_type>::from_promise(*this) };
}

std::suspend_always IEnumerator::promise_type::yield_value(WaitForSeconds wait)
{
    delete currentWait;
    currentWait = new WaitForSeconds(wait);

    return {};
}

IEnumerator::IEnumerator(std::coroutine_handle<promise_type> handle) : handle(handle)
{

}

IEnumerator::IEnumerator(IEnumerator&& other) noexcept : handle(other.handle)
{
    other.handle = nullptr;
}

IEnumerator& IEnumerator::operator=(IEnumerator&& other) noexcept
{
    if (&other != this)
    {
        if (handle)
            handle.destroy();

        handle = other.handle;
        other.handle = nullptr;
    }

    return *this;
}

IEnumerator::~IEnumerator()
{
    if (handle)
        handle.destroy();
}

bool IEnumerator::MoveNext() const
{
    // 핸들이 없거나 완료된 상태라면 중지
    if (!handle || handle.done())
        return false;

    // 현재 대기 상태를 가져옴
    auto& promise = handle.promise();

    if (promise.currentWait)
    {
        if (!promise.currentWait->IsDone())
            return true;

        delete promise.currentWait;
        promise.currentWait = nullptr;
    }

    handle.resume();

    return !handle.done();
}