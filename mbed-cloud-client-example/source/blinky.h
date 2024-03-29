// ----------------------------------------------------------------------------
// Copyright 2018-2020 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#ifndef __BLINKY_H__
#define __BLINKY_H__

#include "sal-stack-nanostack-eventloop/nanostack-event-loop/eventOS_event.h"

class M2MResource;
class MbedCloudClient;
class Commander;

#include <stdint.h>
#include <string>

class Blinky {
    typedef enum {
        STATE_IDLE,
        STATE_STARTED,
    } BlinkyState;

public:
    Blinky();

    ~Blinky();

    void init(MbedCloudClient &client, Commander *commander, M2MResource *resource, long sensor_update_interval, std::string sensor_type);

    bool start(const char *pattern, size_t length, bool pattern_restart);

    void stop();

    void shake(bool enable);
    std::string sensor_type();

public:
    // This needs
    void event_handler(const arm_event_s &event);

    void request_next_loop_event();
    void request_automatic_increment_event();

private:
    void create_tasklet();
    void handle_pattern_event();
    bool request_timed_event(uint8_t event_type, arm_library_event_priority_e priority, int32_t delay);

    void handle_buttons();
    void handle_automatic_increment();

private:
    int get_next_int();
    bool run_step();

private:

    char *_pattern;
    const char *_curr_pattern;

    MbedCloudClient *_client;

    Commander *_commander;

    std::string _sensor_type;

    M2MResource     *_sensed_res;

    int              _sensed_count;

    BlinkyState _state;

    bool _restart;

    static int8_t _tasklet;

    bool _shake;

    long _sensor_update_interval_s;   
};
#endif /* __BLINKY_H__ */
