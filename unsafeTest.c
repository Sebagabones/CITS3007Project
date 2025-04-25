#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char name[50];
	char buffer[100];

	printf("Enter your name: ");
	gets(name);

	strcpy(buffer, name);

	printf("Hello, %s\n", buffer);

	system("ls -la");

	return(0);
}
