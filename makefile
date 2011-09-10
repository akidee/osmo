CC = g++
CFLAGS += -Wall -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -O3 -lmongoclient -lboost_thread -lboost_filesystem -lboost_program_options -lboost_system -lpthread -lexpat -I/Users/andi/.libraries/mongo

bin/osmo:
	$(CC) $(CFLAGS) src/osmo.cc -o bin/osmo

.PHONY : clean
clean:
	rm bin/*
