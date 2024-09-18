// Copyright 2024 Northern.tech AS
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(mender_app, LOG_LEVEL_DBG);

/* This piece of code is heavily inspired by Zephyr OS project sample:
 * https://github.com/zephyrproject-rtos/zephyr/tree/main/samples/net/cloud/tagoio_http_post
 */

#include <assert.h>

#include <zephyr/net/wifi_mgmt.h>

static bool connected;
static struct net_mgmt_event_callback wifi_shell_mgmt_cb;

static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb) {
  const struct wifi_status *status = (const struct wifi_status *)cb->info;

  if (status->status != 0) {
    LOG_ERR("Connection request failed (%d)\n", status->status);
  } else {
    LOG_INF("WIFI Connected\n");
    connected = true;
  }
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint32_t mgmt_event, struct net_if *iface) {
  switch (mgmt_event) {
  case NET_EVENT_WIFI_CONNECT_RESULT:
    handle_wifi_connect_result(cb);
    break;
  default:
    break;
  }
}

void wifi_connect(void) {
  int nr_tries = 20;
  int ret = 0;

  net_mgmt_init_event_callback(&wifi_shell_mgmt_cb, wifi_mgmt_event_handler,
                               NET_EVENT_WIFI_CONNECT_RESULT);

  net_mgmt_add_event_callback(&wifi_shell_mgmt_cb);

  struct net_if *iface = net_if_get_default();
  static struct wifi_connect_req_params cnx_params = {
      .ssid = CONFIG_MENDER_APP_WIFI_SSID,
      .ssid_length = strlen(CONFIG_MENDER_APP_WIFI_SSID),
      .psk = CONFIG_MENDER_APP_WIFI_PSK,
      .psk_length = strlen(CONFIG_MENDER_APP_WIFI_PSK),
      .channel = 0,
      .security = WIFI_SECURITY_TYPE_PSK,
  };

  connected = false;

  LOG_INF("WIFI try connecting to %s...\n", cnx_params.ssid);

  while (nr_tries-- > 0) {
    ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &cnx_params,
                   sizeof(struct wifi_connect_req_params));
    if (ret == 0) {
      break;
    }

    /* Let's wait few seconds to allow wifi device be on-line */
    LOG_ERR("Connect request failed %d. Waiting iface be up...\n", ret);
    k_msleep(500);
  }

  /* Wait for connection */
  while (!connected) {
    k_msleep(100);
  }
}
