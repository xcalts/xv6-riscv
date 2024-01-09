# Λειτουργικά Συστήματα (Κ22) - Εργασία 2η

## Υλοποίηση νέων system calls: `setpriority` και `getpinfo`

1. Πρόσθεσα το `struct stat` στις δηλώσεις που αφορούν στατιστικά στον kernel space. 

```diff
# kernel/stat.h
+ struct pstat {
+  int inuse[NPROC];      // Whether this slot of the process table is in use (1 or 0)
+  int pid[NPROC];        // PID of each process
+  int ppid[NPROC];       // Parent PID of each process
+  int sz[NPROC];         // Process size
+  char state[NPROC][10]; // Process state
+  char name[NPROC][16];  // Process name
+  int priority[NPROC];   // Priority of each process
+};
```

2. Πρόσθεσα definitions των syscalls για kernel space.

```diff
# kernel/defs.h
...
+ struct pstat;
...
+ int             setpriority(int,int);
+ void            getpinfo(struct pstat *);
...
```

3. Πρόσθεσα τις παραμέτρους που αφορούν το `int priority`.

```diff
#  kernel/param.h
...
+ #define DEFAULTPRIORITY 10 // default priority value
+ #define MAXPRIORITY 20     // max priority value
+ #define MINPRIORITY 1      // min priority value
```

4. Αρχικοποιώ την προτεραιότητα στην `allocproc` και πρόσθεσα την λειτουργικότητα των `setpriority` και `getpinfo`.

> Στην `setpriority` πρόσθεσα το όρισμα `pid` που θα με βοηθήσει στο `schedulertest`.

```diff
#  kernel/proc.c
...
static struct proc*
allocproc(void)
{
  ...
+  p->priority = DEFAULTPRIORITY; // [+] Initialize priority
  
  // Allocate a trapframe page.
  if((p->trapframe = (struct trapframe *)kalloc()) == 0){
    freeproc(p);
    ...
}
...
+int 
+setpriority(int priority, int pid) {
+    ...
+}

+void 
+getpinfo(struct pstat *pinfo) {
+  ...
+}
```

Κάθε process πρέπει να έχει και ένα `priority`.

```diff
#  kernel/proc.h
struct proc {
  ...
+ int priority;                // Process priority
};
```

5. Πρόσθεσα αριθμούς για τα syscall που δημιουργώ.

```diff
#  kernel/syscall.h
+ #define SYS_setpriority  22
+ #define SYS_getpinfo     23
```

6. Πρόσθεσα wrappers των syscalls για το user space.

```diff
#  kernel/sysproc.h
+uint64
+sys_setpriority(void) {
+  int priority;
+  int pid;
+  
+  argint(0, &priority);
+  argint(1, &pid);
+
+  return setpriority(priority, pid);
+}
+
+ 
+uint64 sys_getpinfo(void) {
+  struct pstat *pinfo;
+
+  argaddr(0, (uint64*)&pinfo);
+  
+  getpinfo(pinfo);
+
+  return 0;
+}
```

7. Αντιστοίχησα wrappers με syscalls.

```diff
#  kernel/syscall.c
static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
...
+ [SYS_setpriority] sys_setpriority,
+ [SYS_getpinfo] sys_getpinfo,
};
```

8. Πρόσθεσα τις δηλώσεις των syscalls για το user space.

```diff
#  user/user.h
+struct pstat;

// system calls
int fork(void);
...
+int setpriority(int, int); 
+int getpinfo(struct pstat *);
```

9. Χρειάζεται και αυτό για να δημιουργηθούν τα syscalls

```diff
#  user/usys.pl
entry("fork");
...
+entry("setpriority");
+entry("getpinfo");
```

