all: test_work.so

test_work.so: test_work.c
	gcc -Wall -fPIC -shared -o test_work.so test_work.c

clean:
	rm -f *.so
