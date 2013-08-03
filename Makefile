
CMOCKERY_CFLAGS = `pkg-config --cflags cmockery`
CMOCKERY_LIBS = `pkg-config --libs cmockery`

CFLAGS += -Iinclude -Wall


CODERUNNER_SRC = src/coderunner.c
CODERUNNER_HEADER = include/coderunner.h
CODERUNNER_RELEASE_OBJ = $(patsubst src/%.c, src/%.o, $(CODERUNNER_SRC))
CODERUNNER_DEBUG_OBJ = $(patsubst src/%.c, test/%.o, $(CODERUNNER_SRC))

TEST_BINARIES = test/test_coderunner


all: test

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test/%.o: src/%.c
	$(CC) -DDEBUG -g $(CFLAGS) $(CMOCKERY_CFLAGS) -c $< -o $@


test: $(TEST_BINARIES)

test/test_coderunner: test/test_coderunner.c $(CODERUNNER_DEBUG_OBJ) $(CODERUNNER_HEADER)
	$(CC) -g $(CFLAGS) $(CMOCKERY_CFLAGS) $< $(CODERUNNER_DEBUG_OBJ) -o $@ $(CMOCKERY_LIBS)


clean:
	$(RM) $(CODERUNNER_RELEASE_OBJ)
	$(RM) $(CODERUNNER_DEBUG_OBJ)
	$(RM) $(TEST_BINARIES)



# vim: ts=4 sw=4 ai nowrap
