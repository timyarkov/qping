qping:
	gcc qping.c -o qping -lcurl

qp:
	gcc qping.c -o qp -lcurl -DQP_MODE

clean:
	rm -f qping
	rm -f qp