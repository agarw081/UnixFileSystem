test: test.c
	gcc -o test test.c converter.c mini_filesystem.c 
clean: 
	rm test
