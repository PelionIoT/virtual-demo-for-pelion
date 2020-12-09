#include "commander.h"
#include "blinky.h"
#include "json.h"
#include "simplem2mclient.h"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <thread>

#define MQUEUE_CMD "/mqueue-cmd"
#define MQUEUE_RESP "/mqueue-resp"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 1

using json = nlohmann::json;
using namespace std;

Commander::Commander(SimpleM2MClient &client, Blinky &blinky)
    : _client(client), _blinky(blinky) {
  struct mq_attr attr;

  attr.mq_flags = 0;
  attr.mq_maxmsg = MAX_MESSAGES;
  attr.mq_msgsize = MAX_MSG_SIZE;
  attr.mq_curmsgs = 0;

  if ((qd_cmd = mq_open(MQUEUE_CMD, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS,
                        &attr)) == -1) {
    perror("Server: mq_open (qd_cmd)");
    exit(1);
  }

  if ((qd_resp = mq_open(MQUEUE_RESP, O_WRONLY | O_CREAT, QUEUE_PERMISSIONS,
                         &attr)) == -1) {
    perror("Server: mq_open (qd_resp)");
    exit(1);
  }

  cout << "Commander initialized and waiting for cmds..." << endl;
}

void Commander::sendMsg(const char *cmd, const char *params, const char *data) {
  char out_buffer[MSG_BUFFER_SIZE];

  json resp;

  resp["cmd"] = cmd;
  resp["data"] = data;
  if (params)
    resp["params"] = params;

  strcpy(out_buffer, resp.dump().c_str());
  cout << "--> (qd_resp) : " << out_buffer << endl;

  if (mq_send(qd_resp, out_buffer, strlen(out_buffer), 0) == -1) {
    perror("Server: Not able to send response!");
  }

  cout << "--> (qd_resp) sent." << endl;
}

void Commander::listen() {
  std::thread t([&]() {
    char in_buffer[MSG_BUFFER_SIZE];

    while (true) {
      // get the oldest message with highest priority
      ssize_t size;

      if (mq_receive(qd_cmd, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
        perror("Server: mq_receive");
        exit(1);
      }

      // parse json
      auto json_msg = json::parse(in_buffer);
      cout << "<-- (qd_cmd) : " << json_msg.dump() << endl;

      // extract command
      string cmd = json_msg["cmd"];

      if (cmd == "getID") {
        const ConnectorClientEndpointInfo *endpoint =
            _client.get_cloud_client().endpoint_info();

        if (endpoint) {
          sendMsg("getID", NULL, endpoint->internal_endpoint_name.c_str());
        }

      } else if (cmd == "shake") {
        _blinky.shake(json_msg["enable"]);

      } else {
        sendMsg("err", NULL, "unknown command");
      }
    }
  });

  t.detach();
}
