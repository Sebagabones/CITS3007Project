// keep_open.c

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE    1024

char buf[BUF_SIZE];

static void fail(const char *mesg)
{
   perror(mesg);
   exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
   argc--;
   argv++;

   if (argc != 1)
   {
      /*replaced with fprintf(stderr, "%s", "expected 1 arg, FILENAME\n"); */
      fputs("expected 1 arg, FILENAME\n", stderr);
      exit(EXIT_FAILURE);
   }

   printf("opening file\n");
   int fd = open(argv[0], O_RDWR);

   if (fd == -1)
   {
      fail("couldn't open file");
   }

   printf("running 'tail'\n");
   system("tail -f /dev/null");

   // read up to a buffer's worth
   ssize_t read_res = read(fd, buf, BUF_SIZE);

   if (read_res == -1)
   {
      fail("couldn't open file");
   }

   printf("contents read: %s\n", buf);

   close(fd);
}
