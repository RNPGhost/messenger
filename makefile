.PHONY: all

all: messenger_client messenger_server

messenger_client: net.cc client.cc | net.h
	g++ --std=c++1z $^ -o $@ -Wall -Wextra -pedantic

messenger_server: net.cc server.cc | net.h
	g++ -pthread --std=c++1z $^ -o $@ -Wall -Wextra -pedantic

