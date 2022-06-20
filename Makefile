.PHONY: all
all: server client
	@echo build $@

.PHONY: clean
clean:
	rm -f server
	rm -f client 

server: server.c $(OBJS)
	gcc -g -o $@ $< $(OBJS) -lpthread

client: client.c $(OBJS)
	gcc -g -o $@ $< $(OBJS) -lpthread
