PingpongOS: ppos_core.c queue.c
	gcc -Wall -o PPOS ppos_core.c queue.c pingpong-mqueue.c -g -lm

clean:
	@rm -rf *.o PPOS
