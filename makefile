.PHONY: all

all: messenger_client messenger_server

messenger_client: client.cc http.cc net.cc | http.h net.h
	g++ --std=c++1z $^ -o $@ -Wall -Wextra -pedantic

messenger_server: http.cc net.cc server.cc | http.h net.h
	g++ -pthread --std=c++1z $^ -o $@ -Wall -Wextra -pedantic
