#include <stdio.h>

#ifdef ALTERNATE_MAIN
int main()
{
	const char printString[] = "an alternative implementation of main";

	printf("%s\n", printString);

	return(0);
}

#endif
