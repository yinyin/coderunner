
CMOCKERY_CFLAGS = `pkg-config --cflags cmockery`
CMOCKERY_LIBS = `pkg-config --libs cmockery`

CFLAGS += -Iinclude -Wall
CFLAGS_TEST = -DUNIT_TESTING=1 -g $(CFLAGS)

CODERUNNER_SRC = src/coderunner.c
CODERUNNER_HEADER = include/coderunner.h
CODERUNNER_RELEASE_OBJ = $(patsubst src/%.c, src/%.o, $(CODERUNNER_SRC))
CODERUNNER_DEBUG_OBJ = $(patsubst src/%.c, test/%.o, $(CODERUNNER_SRC))

TEST_BINARIES = test/test_coderunner
TEST_DATAEXE = test/td1


all: test

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test/%.o: src/%.c
	$(CC) $(CFLAGS_TEST) $(CMOCKERY_CFLAGS) -c $< -o $@


test/td1: test/td1.c
	$(CC) -o $@ $<


test: $(TEST_DATAEXE) $(TEST_BINARIES)

test/test_coderunner: test/test_coderunner.c $(CODERUNNER_DEBUG_OBJ) $(CODERUNNER_HEADER)
	$(CC) -g $(CFLAGS) $(CMOCKERY_CFLAGS) $< $(CODERUNNER_DEBUG_OBJ) -o $@ $(CMOCKERY_LIBS)



clean:
	$(RM) $(CODERUNNER_RELEASE_OBJ)
	$(RM) $(CODERUNNER_DEBUG_OBJ)
	$(RM) $(TEST_DATAEXE)
	$(RM) $(TEST_BINARIES)



# vim: ts=4 sw=4 ai nowrap
