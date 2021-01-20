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
// distributed under the License is distributed on an "AS ISBASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------
#include "fota/fota_base.h"

#ifdef MBED_CLOUD_CLIENT_FOTA_ENABLE

#define TRACE_GROUP "FOTA"

#include "fota/fota.h"
#include "fota/fota_app_ifs.h"
#include "fota/fota_source.h"
#include "fota/fota_internal.h"
#include "fota/fota_event_handler.h"
#include "fota/fota_component_defs.h"
#include "fota/fota_component.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mqueue.h>

#define MQUEUE_FOTA "/mqueue-fota"
#define QUEUE_PERMISSIONS 0660
#define UNINITIALIZED 99

// the mqueue to send msgs to reflect firmware status
// "Commander" is listening which then forwards to UI
static mqd_t qd_fota = UNINITIALIZED;

void init_queue() {
    if ((qd_fota = mq_open(MQUEUE_FOTA, O_WRONLY | O_CREAT, QUEUE_PERMISSIONS, NULL)) == -1) {
        FOTA_APP_PRINT("mq_open (qd_fota)");
        exit(1);
    }
}

void fota_app_on_download_progress(size_t downloaded_size, size_t current_chunk_size, size_t total_size)
{
    FOTA_ASSERT(total_size);
    static const uint32_t  print_range_percent = 5;

    // init mqueue for IPC
    if (qd_fota == UNINITIALIZED) {
        init_queue();
    }

    total_size /= 100;
    // In case total size is less then 100B return without printing progress
    if (total_size == 0) {
        return;
    }

    uint32_t progress = (downloaded_size + current_chunk_size) / total_size;
    uint32_t prev_progress = downloaded_size / total_size;

    if (downloaded_size == 0 || ((progress / print_range_percent) > (prev_progress / print_range_percent))) {
        FOTA_APP_PRINT("Downloading firmware. %" PRIu32 "%c", progress, '%');        
        // construct msg to send to queue
        char msg[255]; sprintf(msg, "fota_app_on_download_progress,%d", progress);
        // send IPC
        mq_send(qd_fota, msg, strlen(msg), 0);
    }
}

/* Pelion FOTA done or terminated.
 * Application can restore performance sensitive tasks and
 * dismiss any update running dialogs.
 *
*/
int fota_app_on_complete(int32_t status)
{
    return FOTA_STATUS_SUCCESS;
}

/* Pelion FOTA Client wishes to reboot and apply the new firmware.

    The user application is supposed to save all current work
    before rebooting.

    Note: the authorization call can be postponed and called later.
    This doesn't affect the performance of the Cloud Client.
*/
int fota_app_on_install_authorization(uint32_t token)
{
    (void) token; // unused;
    fota_app_authorize_update();
    FOTA_APP_PRINT("Install authorization granted");
    // construct msg to send to queue
    char msg[255]; sprintf(msg, "fota_app_on_install_authorization,%s", "Install authorization granted");
    // send IPC
    mq_send(qd_fota, msg, strlen(msg), 0);
    return FOTA_STATUS_SUCCESS;
}

/* Pelion FOTA Client wishes to download new firmware.
    This can have a negative impact on the performance of the
    rest of the system.

    The user application is supposed to pause performance
    sensitive tasks before authorizing the download.

    Note: the authorization call can be postponed and called later.
    This doesn't affect the performance of the Cloud Client.
*/
int fota_app_on_download_authorization(
    uint32_t token,
    const manifest_firmware_info_t *candidate_info,
    fota_component_version_t curr_fw_version
)
{
    (void) token; // unused;
    char curr_semver[FOTA_COMPONENT_MAX_SEMVER_STR_SIZE] = { 0 };
    char new_semver[FOTA_COMPONENT_MAX_SEMVER_STR_SIZE] = { 0 };
    fota_component_version_int_to_semver(curr_fw_version, curr_semver);
    fota_component_version_int_to_semver(candidate_info->version, new_semver);
    FOTA_APP_PRINT("---------------------------------------------------");
    FOTA_APP_PRINT(
        "Updating component %s from version %s to %s",
        candidate_info->component_name,
        curr_semver, new_semver
    );

    FOTA_APP_PRINT("Update priority %" PRIu32, candidate_info->priority);

    if (candidate_info->payload_format == FOTA_MANIFEST_PAYLOAD_FORMAT_DELTA) {
        FOTA_APP_PRINT(
            "Delta update. Patch size %zuB full image size %zuB",
            candidate_info->payload_size,
            candidate_info->installed_size
        );
    } else {
        FOTA_APP_PRINT("Update size %zuB", candidate_info->payload_size);
    }
    FOTA_APP_PRINT("---------------------------------------------------");
    FOTA_APP_PRINT("Download authorization granted");

    char msg[255]; sprintf(msg, "fota_app_on_download_authorization,%s", "Download authorization granted");
    // send IPC
    mq_send(qd_fota, msg, strlen(msg), 0);

    fota_app_authorize_update();
    /* Application can reject an update in the following way
        fota_app_reject_update(127);
        Reason error code will be logged.
       Alternatively application can defer the update by calling
        fota_app_defer_update();
       Deferred update will be restarted on next boot or by calling fota_app_resume() API.

    */
    return FOTA_STATUS_SUCCESS;
}

#if defined(TARGET_LIKE_LINUX) && !defined(USE_ACTIVATION_SCRIPT)  // e.g. Yocto target have different update activation logic residing outside of the example
// Simplified Linux use case example.
// For MAIN component update the the binary file current process is running.
// Simulate component update by just printing its name.
// After the installation callback returns, FOTA will "reboot" by calling pal_osReboot().

int fota_app_on_install_candidate(const char *candidate_fs_name, const manifest_firmware_info_t *firmware_info)
{
    int ret = FOTA_STATUS_SUCCESS;
    if (0 == strncmp(FOTA_COMPONENT_MAIN_COMPONENT_NAME, firmware_info->component_name, FOTA_COMPONENT_MAX_NAME_SIZE)) {
        // installing MAIN component
        ret = fota_component_install_main(candidate_fs_name);
        if (FOTA_STATUS_SUCCESS == ret) {
            FOTA_APP_PRINT("Successfully installed MAIN component\n");
             // construct msg to send to queue
            char msg[255]; sprintf(msg, "fota_app_on_install_candidate,%s", "New firmware installed");
            // send IPC
            mq_send(qd_fota, msg, strlen(msg), 0);
            // FOTA does support a case where installer method reboots the system.
        }
    } else {
        FOTA_APP_PRINT("%s component installed (example)\n", firmware_info->component_name);
    }
    return ret;
}
#endif // defined(TARGET_LIKE_LINUX) && !defined(USE_ACTIVATION_SCRIPT)

#endif  // MBED_CLOUD_CLIENT_FOTA_ENABLE