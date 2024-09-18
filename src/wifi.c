/*
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/net/wifi_mgmt.h>

static bool connected;
static struct net_mgmt_event_callback wifi_shell_mgmt_cb;

static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_status *status = (const struct wifi_status *)
					   cb->info;

	switch (status->conn_status) {
		case WIFI_STATUS_CONN_SUCCESS:
			printf("Connection success\n");
			break;
		case WIFI_STATUS_CONN_FAIL:
			printf("Connection failed\n");
			break;
		case WIFI_STATUS_CONN_WRONG_PASSWORD:
			printf("Wrong password\n");
			break;
		case WIFI_STATUS_CONN_TIMEOUT:
			printf("Connection timeout\n");
			break;
		case WIFI_STATUS_CONN_AP_NOT_FOUND:
			printf("Access point not found\n");
			break;
		case WIFI_STATUS_CONN_LAST_STATUS:
			printf("Disconnected: ");
			switch (status->disconn_reason)
			{
			case WIFI_REASON_DISCONN_SUCCESS:
				printf("Success, overload status as reason\n");
				break;
			case WIFI_REASON_DISCONN_UNSPECIFIED:
				printf("Unspecified reason\n");
				break;
			case WIFI_REASON_DISCONN_USER_REQUEST:
				printf("Due to user request\n");
				break;
			case WIFI_REASON_DISCONN_AP_LEAVING:
				printf("Due to AP leaving\n");
				break;
			case WIFI_REASON_DISCONN_INACTIVITY:
				printf("Due to inactivity\n");
				break;
			default:
				break;
			}
	}

	if (status->status) {
		printf("Connection request failed (%d)\n", status->status);
	} else {
		printf("WIFI Connected\n");
		connected = true;
	}
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
				    uint32_t mgmt_event, struct net_if *iface)
{
	switch (mgmt_event) {
	case NET_EVENT_WIFI_CONNECT_RESULT:
		handle_wifi_connect_result(cb);
		break;
	default:
		break;
	}
}

void wifi_connect(void)
{
	int nr_tries = 20;
	int ret = 0;

	net_mgmt_init_event_callback(&wifi_shell_mgmt_cb,
				     wifi_mgmt_event_handler,
				     NET_EVENT_WIFI_CONNECT_RESULT);

	net_mgmt_add_event_callback(&wifi_shell_mgmt_cb);

	struct net_if *iface = net_if_get_default();
	static struct wifi_connect_req_params cnx_params = {
		.ssid = CONFIG_TAGOIO_HTTP_WIFI_SSID,
		.ssid_length = 0,
		.psk = CONFIG_TAGOIO_HTTP_WIFI_PSK,
		.psk_length = 0,
		.channel = 0,
		.security = WIFI_SECURITY_TYPE_PSK,
	};

	cnx_params.ssid_length = strlen(CONFIG_TAGOIO_HTTP_WIFI_SSID);
	cnx_params.psk_length = strlen(CONFIG_TAGOIO_HTTP_WIFI_PSK);

	connected = false;

	printf("WIFI try connecting to %s...\n", CONFIG_TAGOIO_HTTP_WIFI_SSID);

	while (nr_tries-- > 0) {
		ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &cnx_params,
			       sizeof(struct wifi_connect_req_params));
		if (ret == 0) {
			break;
		}

		/* Let's wait few seconds to allow wifi device be on-line */
		printf("Connect request failed %d. Waiting iface be up...\n", ret);
		k_msleep(500);
	}

	while (!connected) {
		/* Wait for connection */
		k_msleep(100);
	}
}
