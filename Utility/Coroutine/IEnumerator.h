#pragma once

#include <coroutine>
#include <memory>
#include <utility>

#include "WaitTime/WaitForSeconds.h"

struct IEnumerator
{
	// 코루틴의 상태를 관리하는 promise_type 구조체
    struct promise_type
    {
        ~promise_type();

        IEnumerator get_return_object();

		std::suspend_always initial_suspend() { return {}; }        // 코루틴이 시작될 때 즉시 대기 상태로 진입하도록 설정
		std::suspend_always final_suspend() noexcept { return {}; } // 코루틴이 종료될 때 즉시 대기 상태로 진입하도록 설정

        void return_void() {}
        void unhandled_exception() {}

        std::suspend_always yield_value(WaitForSeconds wait);
        
        // 현재 대기 상태 정보를 저장하는 포인터
        WaitForSeconds* currentWait = nullptr;
    };

    explicit IEnumerator(std::coroutine_handle<promise_type> handle);

    // 복사 금지 및 이동 허용
    IEnumerator(const IEnumerator&) = delete;
    IEnumerator& operator=(const IEnumerator&) = delete;
    IEnumerator(IEnumerator&& other) noexcept;
    IEnumerator& operator=(IEnumerator&& other) noexcept;

    ~IEnumerator();

	// 현재 대기 상태가 완료되었는지 확인하고, 완료되었다면 다음 단계로 진행합니다.
    bool MoveNext() const;

    // 코루틴 핸들을 저장하는 멤버 변수
    std::coroutine_handle<promise_type> handle;
};