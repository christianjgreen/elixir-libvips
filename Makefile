
ERLANG_PATH = $(shell erl -eval 'io:format("~s", [lists:concat([code:root_dir(), "/erts-", erlang:system_info(version), "/include"])])' -s init stop -noshell)
CFLAGS += -pthread -O3 -Wall -g -I$(ERLANG_PATH)
VIPS = `pkg-config vips-cpp --cflags --libs`
LIBS = $(VIPS)

all:
	gcc --std=c++11 -undefined dynamic_lookup -dynamiclib -o src/vips.so src/vips.cc $(CFLAGS) $(LIBS)

clean:
	rm  -r "priv/vips.so"