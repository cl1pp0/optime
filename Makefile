all:
	$(CC) -Wall -o optime -lrt optime.c
	$(CC) -Wall -o getoptime getoptime.c

start:
	./optime

check:
	ps xj | head -1
	@ps xj | grep optime

log:
	grep optime /var/log/syslog | tail
