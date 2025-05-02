// unsafe_example.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void unsafe_gets()
{
	char buf[64];

	printf("Enter some text (unsafe gets): ");
	gets(buf);
	printf("You entered: %s\n", buf);
}

void unsafe_strcpy()
{
	char src[128];
	char dest[64];

	printf("Enter text to copy (unsafe strcpy): ");
	fgets(src, sizeof(src), stdin);
	strcpy(dest, src);
	printf("Copied: %s\n", dest);
}

void unsafe_sprintf()
{
	char name[64];
	char msg[128];

	printf("Enter your name (unsafe sprintf): ");
	fgets(name, sizeof(name), stdin);
	sprintf(msg, "Hello, %s", name);
	printf("%s\n", msg);
}

void unsafe_scanf()
{
	int	 num;
	char name[32];

	printf("Enter your name and a number (unsafe scanf): ");
	scanf("%s %d", name, &num);
	printf("Name: %s, Number: %d\n", name, num);
}

void unsafe_format_string()
{
	char input[128];

	printf("Enter a string (unsafe printf): ");
	fgets(input, sizeof(input), stdin);
	printf(input);
	printf("\n");
}

int main()
{
	unsafe_gets();
	unsafe_strcpy();
	unsafe_sprintf();
	unsafe_scanf();
	unsafe_format_string();

	return(0);
}
