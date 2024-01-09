#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"


#define NFORK 20
#define IO 5

int main() {
  int n, pid;
  for(n=0; n < NFORK;n++) {
      pid = fork();
      if (pid < 0)
          break;
      if (pid == 0) {
            for (volatile int i = 0; i < 1000000000; i++) {} // CPU bound process 

          printf("\n[+] Process:%d with Priority: %d finished", n, NFORK - n);
          exit(0);
      } else {
        setpriority(NFORK - n, pid); // Will only matter for PBS, set lower priority for IO bound processes 
      }
  }
  
  exit(0);
}