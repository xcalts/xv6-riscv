#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct pstat pinfo;

  getpinfo(&pinfo);

  printf("name state pid ppid size priority\n");
  printf("================================\n");

  for(int i = 0; i < NPROC; i++)
  {
    if(pinfo.inuse[i] == 1) {
      printf("%s %s %d %d %d %d\n",
        &pinfo.name[i],
        &pinfo.state[i],
        pinfo.pid[i],
        pinfo.ppid[i],
        pinfo.sz[i],
        pinfo.priority[i]);
    }
  }

  exit(0);
}
