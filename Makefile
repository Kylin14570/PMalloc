#obj = build/main.o build/cache.o build/mmap.o build/OffsetPointerConvert.o build/PMalloc.o build/root.o build/sizeclass.o build/SuperBlock.o build/thread.o
#main: $(obj)
#	gcc $(obj) -o build/main -lpmem
#build/%.o: src/%.c src/def.h
#	gcc -c $< -o $@
#clean:
#	rm -rf $(obj) build/main
#run:
#	make
#	build/main

src = $(wildcard src/*.c)
obj = $(patsubst src/%.c, obj/%.o, $(src))
main: $(obj)
	gcc $(obj) -o main -lpmem -lpthread
obj/%.o: src/%.c src/def.h
	gcc -c $< -o $@
.PHONY:clean
clean:
	rm -rf $(obj) main
.PHONY:run
run:
	make
	./main 16 100 100
.PHONY:check
check:
	make
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt
	rm pool
	./main 16 1000 1000 > log.txt
	check/check < log.txt