#include <stdio.h>
#include <stdlib.h>

static size_t fibonacci(size_t numOfFibs, size_t currentNumOfFigs, size_t fst, size_t sec)
{
	/* printf("%zu, %zu\n",fst, sec); */

	size_t b = fst + sec;

	if (currentNumOfFigs != numOfFibs)
	{
		size_t a = sec;
		b = fibonacci(numOfFibs, currentNumOfFigs + 1, a, b);
	}

	return(b);
}

static size_t fibonacciInit(size_t n)
{
	return(fibonacci(n - 2, 0, 0, 1));
}

int main(int argc, const char *argv[])
{
	/* FYI this is zero indexed */
	if (argc != 2)
	{
		printf("Please specify a input number");
		exit(-1);
	}

	size_t input = strtoul(argv[1], NULL, 10);

	if (input < 2)
	{
		printf("cmon bud, first and second Fibonacci numbers are 0, and 1 \n");
		exit(-1);
	}

	size_t returnd = fibonacciInit(input);
	printf("%zu\n", returnd);

	return(0);
}
