/******************************************************************************
 *
 *  Copyright 2023 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of NXP nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
#define LOG_TAG "emvco_tda_test_sm"

/**
 * @brief  This class is responsible for storing the current state class
 *         and calls the next state based on request
 *
 **/

#include "StateMachine.h"
#include "DiscoverState.h"
#include <log/log.h>

StateMachine *StateMachine::mStateMachine = nullptr;
TDAState *StateMachine::currentTDAState = nullptr;

/**
 * @brief  creates the object of StateMachine and stores the
 *         current state as Discover on initialization
 *
 **/
StateMachine::StateMachine() {
  ALOGI("%s \n", __func__);
  currentTDAState = &DiscoverState::getInstance();
}

/**
 * @brief  Returns the singleton object the StateMachine class
 *
 **/
StateMachine &StateMachine::getInstance() {
  if (mStateMachine == nullptr) {
    mStateMachine = new StateMachine();
  }
  return *mStateMachine;
}

/**
 * @brief  caller of this function uses to specify the next state.
 *         It stores as incoming state as current state and request
 *         the current state to perform action on entry
 *
 **/
void StateMachine::setState(TDAState *tdaState) {
  ALOGI("%s \n", __func__);
  currentTDAState = tdaState;
  currentTDAState->enter();
}

/**
 * @brief  Returns the the current TDA state
 *
 **/
TDAState *StateMachine::getCurrentTDAState() { return currentTDAState; };