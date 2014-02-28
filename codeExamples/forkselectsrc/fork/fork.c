#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
	printf("Before fork()\n");
	fork();
	printf("After fork()\n");
	exit(0);
}
