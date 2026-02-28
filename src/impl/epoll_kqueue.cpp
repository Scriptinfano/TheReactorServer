#ifdef __APPLE__

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

int epoll_create(int size) {
    return epoll_create1(0);
}

int epoll_create1(int flags) {
    int kq = kqueue();
    if (kq < 0) return -1;
    if (flags & EPOLL_CLOEXEC) {
        fcntl(kq, F_SETFD, FD_CLOEXEC);
    }
    return kq;
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    struct kevent kev[2];
    int n = 0;
    int flags = 0;

    if (op == EPOLL_CTL_ADD || op == EPOLL_CTL_MOD) {
        flags = EV_ADD | EV_ENABLE;
        if (event->events & EPOLLET) {
            flags |= EV_CLEAR;
        }

        // Handle Read
        if (event->events & EPOLLIN) {
            EV_SET(&kev[n++], fd, EVFILT_READ, flags, 0, 0, event->data.ptr);
        } else if (op == EPOLL_CTL_MOD) {
            EV_SET(&kev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        }

        // Handle Write
        if (event->events & EPOLLOUT) {
            EV_SET(&kev[n++], fd, EVFILT_WRITE, flags, 0, 0, event->data.ptr);
        } else if (op == EPOLL_CTL_MOD) {
            EV_SET(&kev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        }
    } else if (op == EPOLL_CTL_DEL) {
        EV_SET(&kev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        EV_SET(&kev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        n = 2;
    } else {
        errno = EINVAL;
        return -1;
    }
    
    for (int i = 0; i < n; i++) {
        kevent(epfd, &kev[i], 1, NULL, 0, NULL);
    }
    return 0;
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
    struct timespec ts;
    struct timespec *tsp = NULL;

    if (timeout >= 0) {
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = (timeout % 1000) * 1000000;
        tsp = &ts;
    }

    std::vector<struct kevent> kevents(maxevents);
    int n = kevent(epfd, NULL, 0, kevents.data(), maxevents, tsp);

    if (n < 0) return -1;

    for (int i = 0; i < n; i++) {
        events[i].events = 0;
        events[i].data.ptr = kevents[i].udata;

        if (kevents[i].filter == EVFILT_READ) {
            events[i].events |= EPOLLIN;
        } else if (kevents[i].filter == EVFILT_WRITE) {
            events[i].events |= EPOLLOUT;
        }

        if (kevents[i].flags & EV_EOF) {
            events[i].events |= EPOLLHUP | EPOLLRDHUP;
        }
        if (kevents[i].flags & EV_ERROR) {
            events[i].events |= EPOLLERR;
        }
    }

    return n;
}

#endif
