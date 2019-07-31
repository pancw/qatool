INSTALLDIR=$(shell pwd)/../../extlib/install
ROOT=$(shell pwd)

BIN=node_qatool

DEPS=$(shell find ./ -name "*.h")
C_SRC=$(shell find ./ -name "*.c")
CPP_SRC=$(shell find ./ -name "*.cpp") \
	$(ROOT)/../common/module/connectionBase.cpp

OBJ=$(C_SRC:%.c=%.o) $(CPP_SRC:%.cpp=%.o)

INCLUDE= \
	-I $(ROOT)/include \
	-I $(ROOT)/../common/include \
	-I $(INSTALLDIR)/lua51/include \
	-I $(INSTALLDIR)/libevent/include

LIB= \
	$(INSTALLDIR)/lua51/lib/liblua.a \
	$(INSTALLDIR)/libevent/lib/libevent.a \
	$(INSTALLDIR)/libevent/lib/libevent_core.a \
	$(INSTALLDIR)/libevent/lib/libevent_extra.a \
	$(INSTALLDIR)/libevent/lib/libevent_pthreads.a


$(BIN): $(OBJ) $(LIB)
	g++ -o $@ $^ $(LIB) $(INCLUDE) -g -Wall -Wl,-Bstatic -lstdc++ -lc -lrt -Wl,-Bdynamic -ldl

%.o: %.c $(DEPS)
	gcc -c $< -o $@ $(INCLUDE) -Wall

%.o: %.cpp $(DEPS)
	g++ $< -o $@ $(INCLUDE) -c -Wall -std=c++11
#-c 不可或缺

clean:
	rm -rf $(OBJ) $(BIN)
