PingpongOS: ppos_core.c queue.c
	gcc -Wall -o PPOS ppos_core.c queue.c pingpong-join.c -g

clean:
	@rm -rf *.o PPOS
