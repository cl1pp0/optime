# optime
Count operation time of your personal computer (similar to uptime, but w/o suspend phases)

A fiddled personal gimmick comprising:

* fork()
* syslog()
* POSIX timers
* Unix domain sockets IPC

## optime daemon

* optime daemon counts the operation time, i.e. the seconds the computer is running, via POSIX timers
* daemon code is based on [daemon-skeleton-linux-c by pasce](https://github.com/pasce/daemon-skeleton-linux-c)

Start daemon:
```
$ ./optime
```

Terminate daemon:
```
$ killall optime
```

## optime client

* optime client gets the current operation time via UDS IPC from the optime daemon and prints it to STDOUT

Get current operation time:
```
$ ./getoptime
```

