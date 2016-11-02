PingpongOS: ppos_core.c queue.c
	gcc -Wall -o PPOS ppos_core.c queue.c pingpong-tasks1.c -g

clean:
	@rm -rf *.o PPOS
