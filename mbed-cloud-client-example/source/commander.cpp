#include "Commander.h"

#include <fcntl.h>
#include <mqueue.h>
#include <thread>

#define MQUEUE_CMD "/mqueue-cmd"
#define MQUEUE_RESP "/mqueue-resp"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 1

void Commander::init(SimpleM2MClient &client) { _client = &client; }

void Commander::listen() {
  mqd_t qd_cmd, qd_resp; // queue descriptors
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

  char in_buffer[MSG_BUFFER_SIZE];
  char out_buffer[MSG_BUFFER_SIZE];

  std::thread t([]() {
    while (true) {
      // get the oldest message with highest priority
      ssize_t size;

      if (mq_receive(qd_cmd, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
        perror("Server: mq_receive");
        exit(1);
      }

      printf("<-- '%s' \n", in_buffer);
      // send reply message to client
      strcpy(out_buffer, in_buffer);

      if (mq_send(qd_resp, out_buffer, strlen(out_buffer) + 1, 0) == -1) {
        perror("Server: Not able to send response");
        continue;
      }

      printf("response sent to client.\n");
    }
  });

  t.detach();
}
