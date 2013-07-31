
CMOCKERY_CFLAGS = `pkg-config --cflags cmockery`
CMOCKERY_LIBS = `pkg-config --libs cmockery`

CFLAGS += -Iinclude -Wall

CODERUNNER_OBJ = src/coderunner.o
CODERUNNER_HEADER = include/coderunner.h

TEST_BINARIES = test/test_coderunner



test: $(TEST_BINARIES)

test/test_coderunner: test/test_coderunner.c $(CODERUNNER_OBJ) $(CODERUNNER_HEADER)
	$(CC) $(CFLAGS) $(CMOCKERY_CFLAGS) $< $(CODERUNNER_OBJ) -o $@ $(CMOCKERY_LIBS)



clean:
	$(RM) $(CODERUNNER_OBJ)
	$(RM) $(TEST_BINARIES)



# vim: ts=4 sw=4 ai nowrap
