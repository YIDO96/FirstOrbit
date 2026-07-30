#include "pch.h"
#include "TimeManager.h"

// static 변수 초기화
int32 TimeManager::TimerIdGenerator = 0;

void TimeManager::Init()
{
	::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&_frequency));
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&_prevCount)); // CPU 클럭
}

void TimeManager::Update()
{
	uint64 currentCount;
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentCount));

	_deltaTime = (currentCount - _prevCount) / static_cast<float>(_frequency);
	_prevCount = currentCount;

	// dt 클램프: 스파이크(스톨 뒤 큰 dt)가 물리/충돌을 깨뜨리지 않도록 상한을 둔다.
	// GetDT()를 쓰는 모든 소비자(World/UI/타이머)가 자른 값을 받게 여기서 한 번만 처리한다.
	if (_deltaTime > GMaxDeltaTime)
		_deltaTime = GMaxDeltaTime;

	// 프레임율(FPS 계산을 위해)
	_frameCount++;
	_frameTime += _deltaTime;	// 시간을 누적

	// 초당 프레임률 계산
	if (_frameTime >= 1.0f)
	{
		_fps = _frameCount;
		_frameTime -= 1.0f;
		_frameCount = 0;
	}

	// 제거 리스트를 "발사보다 먼저" 처리한다.
	// (반대로 하면, 이전 프레임에 죽은 소유자가 Remove()를 요청해도
	//  이번 프레임 발사 루프에서 한 번 더 실행되어 해제된 메모리를 참조하게 된다)
	_timers.erase(std::remove_if(_timers.begin(), _timers.end(),
		[&](const Timer& timer) {
			return _removeTimers.find(timer.GetId()) != _removeTimers.end();
		}), _timers.end());
	_removeTimers.clear();

	// 추가 리스트
	// 이번 프레임에 추가되어야 하는 타이머들
	_timers.insert(_timers.end(), _addTimers.begin(), _addTimers.end());
	_addTimers.clear();

	// 타이머 업데이트
	for (auto& iter : _timers)
	{
		iter.Update(_deltaTime);
	}
}

// 타이머는 매프레임 Update 로직 호출중이라,
// 안전하게 모든 Update로직이 끝난후에 timer list 에 넣는다.
int32 TimeManager::AddTimer(TimerFunc func, float interval, bool loop)
{
	int32 id = TimerIdGenerator++;
	Timer timer(id, loop, func, interval);
	_addTimers.push_back(timer);
	return id;
}

// 타이머는 매프레임 Update 로직 호출중이라,
// 안전하게 모든 Update로직이 끝난후에 timer list 에서 제거한다.
void TimeManager::Remove(int32 id)
{
	_removeTimers.insert(id);
}

//----------------------------------
// 타이머 객체
//----------------------------------
void Timer::Update(float deltaTime)
{
	_sumTime += deltaTime;
	if (_sumTime >= _interval)
	{
		_func();

		if (_loop)
		{
			_sumTime -= _interval;
		}
	}
}

bool Timer::IsExpired()
{
	return (_sumTime >= _interval);
}
