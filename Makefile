.PHONY : clean all

all: iob

iob:
	gcc iob.c -o iob -lpthread
test:
	./iob

.PHONY : clean
clean:
	rm -f iob *.o test.data

