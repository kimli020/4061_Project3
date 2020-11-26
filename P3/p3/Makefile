CC = gcc
CFLAGS = -D_REENTRANT
LDFLAGS = -lpthread -pthread

web_server: server.c
	${CC} -Wall -o web_server server.c util.o ${CFLAGS} ${LDFLAGS}

clean:
	rm web_server web_server_log
