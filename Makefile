
dwmStatus: status.c
	gcc -o dwmStatus status.c -lX11

clean:
	rm dwmStatus
