// privileged.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
   int fd;

   // Assume that /etc/zzz is an important system file,
   // and it is owned by root with permission 0644.
   // Before running this program, you should create
   // the file /etc/zzz first.
   uid_t spot1_ruid = getuid();
   uid_t spot1_euid = geteuid();

   printf("at spot1: ruid is: %d\n", spot1_ruid);
   printf("at spot1: euid is: %d\n", spot1_euid);
   fd = open("/etc/zzz", O_RDWR | O_APPEND);

   if (fd == -1)
   {
      printf("Cannot open /etc/zzz\n");
      exit(0);
   }

   uid_t spot1_ruid1 = getuid();
   uid_t spot1_euid1 = geteuid();
   printf("at spot1: ruid is: %d\n", spot1_ruid1);
   printf("at spot1: euid is: %d\n", spot1_euid1);
   // Simulate the tasks conducted by the program
   sleep(1);

   // After the task, the root privileges are no longer needed,
   // it's time to relinquish the root privileges permanently.
   setuid(getuid()); // getuid() returns the real uid
   uid_t spot1_ruid2 = getuid();
   uid_t spot1_euid2 = geteuid();
   printf("at spot1: ruid is: %d\n", spot1_ruid2);
   printf("at spot1: euid is: %d\n", spot1_euid2);

   if (fork())  // In the parent process
   {
      close(fd);
      uid_t spot1_ruid3 = getuid();
      uid_t spot1_euid3 = geteuid();
      printf("at spot1: ruid is: %d\n", spot1_ruid3);
      printf("at spot1: euid is: %d\n", spot1_euid3);
      exit(0);
   }
   else    // in the child process
   // perform unprivileged tasks

   // Now, assume that the child process is compromised, malicious
   // attackers have injected the following statements
   // into this process

   {
      write(fd, "Malicious Data\n", 15);
      close(fd);
      uid_t spot1_ruid4 = getuid();
      uid_t spot1_euid4 = geteuid();
      printf("at spot1: ruid is: %d\n", spot1_ruid4);
      printf("at spot1: euid is: %d\n", spot1_euid4);
   }
}
