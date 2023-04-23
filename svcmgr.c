#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

// see also: https://www.devever.net/~hl/usbnkro
// https://github.com/librerpi/rpi-tools/blob/master/keyboard-proxy/main.c
// #define KEY_POWER		116	/* SC System Power Down */


#define MAX_EVENTS 10

int main(int argc, char **argv) {

  bool is_running = false;

  struct epoll_event events[MAX_EVENTS];
  int fd = open("/dev/input/event0", O_RDWR);
  //assert(fd >= 0);

  int ret = ioctl(fd, EVIOCGRAB, (void*)1);
  assert(ret == 0);

  int epollfd = epoll_create1(0);
  assert(epollfd >= 0);

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = fd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev)) {
    perror("unable to epoll add");
    return 1;
  }

  printf("svcmgr started ...\n");
  while (1) {
    int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if (nfds < 0) {
      printf("%d ", nfds);
      perror("epoll_wait failed");
      return 2;
    }

    for (int i=0; i < nfds; i++) {
      struct input_event ev[64];
      int rd = read(events[i].data.fd, ev, sizeof(ev));
      assert(rd > ((signed int)sizeof(struct input_event)));

      for (int j=0; j < rd / ((signed int)sizeof(struct input_event)); j++) {
        unsigned int type, code;
        type = ev[j].type;
        code = ev[j].code;
        //printf("Event: time %ld.%06ld, ", ev[j].time.tv_sec, ev[j].time.tv_usec);

        if (type == EV_KEY && code == KEY_POWER && ev[j].value == 0) {

          if ( is_running == false ) {
            is_running = true;
            printf("SVC : resume\n");

          } else {
            is_running = false;
            printf("SVC : suspend\n");

          }
 
        } /* if */
      } /* for */
    } /* for */
  } /* while */

  return 0;
}
