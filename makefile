PROGRAM=yawn
FLAGS=-Wall -O2 -s $(PROGRAM).c -o $(PROGRAM)

# Make sure you've installed libcurl and zlib
# Install development branch which includes necessary headers
LIBS=-lcurl -lz

all:
	@echo 'Run "make build" to build'
	@echo 'Run "make clean" to delete the compiled binary'

build:
	$(CC) $(FLAGS) $(LIBS)

clean:
	@rm $(PROGRAM)
