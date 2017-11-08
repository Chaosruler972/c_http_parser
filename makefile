client: client.c
	gcc -Wall -Wvla -g client.c -o client
clean: 
	rm client
