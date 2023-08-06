all: server client

CXX=g++
CXXFLAGS=-Wall  
SRCDIR=src/
BINDIR=bin/
INCLUDEDIR=include/
SERVER_OBJS=$(SRCDIR)server.o
CLIENT_OBJS=$(SRCDIR)client.o

server: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)$@ $(SERVER_OBJS) 

client: $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BINDIR)$@ $(CLIENT_OBJS) -lcurl

clean:
	$(RM) $(SRCDIR)*.o
	$(RM) $(INCLUDEDIR)*.o
	$(RM) $(BINDIR)*
