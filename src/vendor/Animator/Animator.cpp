#include "Animator.h"

uint32_t Animator::_timeStep = 10;
bool Animator::_isInitialized = false;
std::vector<Animator*> Animator::_animators;

Animator::Animator() {
    if (!_isInitialized) {
        _isInitialized = true;
#ifdef ESP32
        xTaskCreate(_pollTask, "animator", 8192, NULL, 1, NULL);
#else
        Timer.setInterval(_timeStep, _pollTask);
#endif
    }
    _animators.push_back(this);
}

Animator::Animator(const Animator& other) {
    _duration = other._duration;
    _delay = other._delay;
    _current = other._current;
    _target = other._target;
    _isRunning = other._isRunning;
    _endCallback = other._endCallback;
    _animators.push_back(this);
}

Animator::Animator(Animator&& other) {
    _duration = other._duration;
    _delay = other._delay;
    _current = other._current;
    _target = other._target;
    _isRunning = other._isRunning;
    _endCallback = other._endCallback;
    _animators.push_back(this);
}

Animator::~Animator() {
    auto it = std::find(_animators.begin(), _animators.end(), this);
    if (it != _animators.end()) {
        _animators.erase(it);
    }
}

Animator& Animator::setDuration(const uint32_t& duration) {
    _duration = duration;
    return *this;
}

uint32_t Animator::getDuration() {
    return _duration;
}

Animator& Animator::setDelay(const uint32_t& delay) {
    _delay = delay;
    return *this;
}

uint32_t Animator::getDelay() {
    return _delay;
}

Animator& Animator::setCurrentValue(const float& current) {
    _current = current;
    return *this;
}

float Animator::getCurrentValue() {
    return _current;
}

void Animator::animate(const float& target) {
    onAnimationStart();
    if (_current == target || _duration == 0) {
        onAnimationUpdate(target);
        onAnimationEnd();
        return;
    }
    _target = target;
    _startTime = millis() + _delay;
    _step = (_target - _current) * (float)_timeStep / (float)_duration;
    _isIncreasing = _target > _current;
    _isRunning = true;
}

Animator& Animator::withEnd(VoidCallback callback) {
    _endCallback = callback;
    return *this;
}

void Animator::cancel() {
    _isRunning = false;
    onAnimationCancel();
}

void Animator::_run(const uint32_t& currentTime) {
    if (_isRunning) {
        if (currentTime >= _startTime) {
            _current += _step;
            if ((_isIncreasing && _current >= _target) || (!_isIncreasing && _current <= _target)) {
                _current = _target;
            }
            onAnimationUpdate(_current);

            if (_current == _target) {
                _isRunning = false;
                onAnimationEnd();
                _endCallback();
                _endCallback = [] {};
            }
        }
    }
}

#ifdef ESP32
void Animator::_pollTask(void*) {
    while (true) {
        uint32_t currentTime = millis();
        for (auto animator : _animators) {
            animator->_run(currentTime);
        }
        delay(_timeStep);
    }
}
#else
void Animator::_pollTask() {
    uint32_t currentTime = millis();
    for (auto animator : _animators) {
        animator->_run(currentTime);
    }
}
#endif
