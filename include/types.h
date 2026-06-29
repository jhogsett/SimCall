#ifndef SIMCALL_TYPES_H
#define SIMCALL_TYPES_H

#include <Arduino.h>

using NonBlockingAction = void (*)(uint32_t data);
using NonBlockingAbort = bool (*)(uint32_t data);

class NonBlockingTimer
{
public:
  void start(uint32_t duration)
  {
    _start_time = millis();
    _duration = duration;
  }

  bool elapsed()
  {
    return (millis() - _start_time) >= _duration;
  }

private:
  uint32_t _start_time;
  uint32_t _duration;
};

class NonBlockingLoop
{
public:
  void start(uint32_t cycles)
  {
    _cycles = cycles;
    _cycle = 0;
  }

  bool active()
  {
    return _cycle < _cycles;
  }

  bool last_cycle()
  {
    return _cycle == _cycles - 1;
  }

  bool cycle()
  {
    _cycle += 1;
    return active();
  }

  uint32_t cycle() const { return _cycle; }

private:
  uint32_t _cycles;
  uint32_t _cycle;
};

class NonBlockingSequence
{
public:
  NonBlockingSequence(NonBlockingAction* actions, int* times, int num_actions, bool quick_last_action = false, NonBlockingAbort abort_action = NULL){
    _actions = actions;
    _times = times;
    _num_actions = num_actions;
    _abort_action = abort_action;
    _quick_last_action = quick_last_action;
  }

  void start(int num_cycles, uint32_t data = 0L){
    _num_cycles = num_cycles;
    _data = data;
    restart();
  }

  void restart(){
    _action_iter = 0;
    _state = STATE_START_LOOP;
    _running = false;
    _aborted = false;
  }

  bool aborted(){
    return _aborted;
  }

  bool quick_last_action(){
    return _quick_last_action && _loop.last_cycle() && _action_iter == _num_actions - 1;
  }

  bool step(){
    if(_abort_action && _abort_action(0)){
      _state = STATE_ABORTED;
    }

    switch(_state){
      case STATE_START_LOOP:
        _loop.start(_num_cycles);
        _running = true;
        _state = STATE_START_CYCLE;
        break;

      case STATE_START_CYCLE:
        _action_iter = 0;
        _state = STATE_START_ACTION;
        break;

      case STATE_START_ACTION:
        _action = _actions[_action_iter];
        if(_action != nullptr){
          _action(_data);
        }

        // end the loop if requested to skip last cycle and last action timing 
        if(quick_last_action()){
          _state = STATE_END_LOOP;
          break;
        }

        _time = _times[_action_iter];
        _timer.start(_time);
        _state = STATE_RUNNING;
        break;      

      case STATE_RUNNING:
        if(_timer.elapsed()){
          _state = STATE_NEXT_ACTION;
        }
        break;

      case STATE_NEXT_ACTION:
        _action_iter += 1;
        _state = (_action_iter >= _num_actions) ? STATE_NEXT_CYCLE : STATE_START_ACTION;
        break;

      case STATE_NEXT_CYCLE:
        _state = _loop.cycle() ? STATE_START_CYCLE : STATE_END_LOOP;
        break;

      case STATE_END_LOOP:
        _running = false;
        break;

      case STATE_ABORTED:
        _aborted = true;
        _running = false;
        break;
    }

    return _running;
  }

  enum State : byte {
    STATE_START_LOOP,
    STATE_START_CYCLE,
    STATE_START_ACTION,
    STATE_RUNNING,
    STATE_NEXT_ACTION,
    STATE_NEXT_CYCLE,
    STATE_END_LOOP,
    STATE_ABORTED
  };

private:
  NonBlockingAction* _actions;
  int* _times;
  int _num_actions;
  int _num_cycles;
  uint32_t _data;
  int _action_iter;
  NonBlockingAbort _abort_action;
  bool _quick_last_action;

  State _state;
  NonBlockingLoop _loop;
  NonBlockingAction _action;
  int _time;
  NonBlockingTimer _timer;
  bool _running;
  bool _aborted;
};

#endif