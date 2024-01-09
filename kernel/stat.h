#include "param.h"

#define T_DIR     1   // Directory
#define T_FILE    2   // File
#define T_DEVICE  3   // Device

struct stat {
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short type;  // Type of file
  short nlink; // Number of links to file
  uint64 size; // Size of file in bytes
};

 struct pstat {
  int inuse[NPROC];      // Whether this slot of the process table is in use (1 or 0)
  int pid[NPROC];        // PID of each process
  int ppid[NPROC];       // Parent PID of each process
  int sz[NPROC];         // Process size
  char state[NPROC][10]; // Process state
  char name[NPROC][16];  // Process name
  int priority[NPROC];   // Priority of each process
};