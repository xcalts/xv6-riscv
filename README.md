# Λειτουργικά Συστήματα (Κ22) - Εργασία 2η

1. [Υλοποίηση νέων system calls: `setpriority` και `getpinfo`](#υλοποίηση-νέων-system-calls-setpriority-και-getpinfo)

2. [Υλοποίηση προγράμματος χρήστη `ps`](#υλοποίηση-προγράμματος-χρήστη-ps)

3. [Υλοποίηση priority based scheduler](#υλοποίηση-priority-based-scheduler)

4. [usertests](#usertests)

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

5. Κάθε process πρέπει να έχει και ένα `priority`.

```diff
#  kernel/proc.h
struct proc {
  ...
+ int priority;                // Process priority
};
```

6. Πρόσθεσα αριθμούς για τα syscall που δημιουργώ.

```diff
#  kernel/syscall.h
+ #define SYS_setpriority  22
+ #define SYS_getpinfo     23
```

7. Πρόσθεσα wrappers των syscalls για το user space.

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

8. Αντιστοίχησα wrappers με syscalls.

```diff
#  kernel/syscall.c
static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
...
+ [SYS_setpriority] sys_setpriority,
+ [SYS_getpinfo] sys_getpinfo,
};
```

9. Πρόσθεσα τις δηλώσεις των syscalls για το user space.

```diff
#  user/user.h
+struct pstat;

// system calls
int fork(void);
...
+int setpriority(int, int); 
+int getpinfo(struct pstat *);
```

10. Χρειάζεται και αυτό για να δημιουργηθούν τα syscalls

```diff
#  user/usys.pl
entry("fork");
...
+entry("setpriority");
+entry("getpinfo");
```

## Υλοποίηση προγράμματος χρήστη `ps`

1. Πρόσθεσα την λειτουργικότητα της `ps`.

```diff
#  user/ps.c
+#include "kernel/types.h"
+#include "kernel/stat.h"
+#include "user/user.h"
+
+int
+main(int argc, char *argv[])
+{
+  ...
+}
```

2. Για να γίνει compiled και επομένως διαθέσιμο στον user:

```diff
#  ./Makefile
+UPROGS=\
	$U/_cat\
 ...
+	$U/_ps\
...
```

## Υλοποίηση priority based scheduler

1. Άλλαξα την συνάρτητη scheduler για να δίνει προτεραιότητα με βάση το priority.

> Απλά βρίσκω το μεγαλύτερο `priority`, δεν έκανα RR γιατί θα γινόταν πάρα πολύ περίπλοκο 😂.

```diff
# kernel/proc.c
...
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  
  c->proc = 0;
  for(;;){
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();
+
+   struct proc* process = 0;
+    int hp = -1;
+    for (p = proc; p < &proc[NPROC]; p++)
+    {
+      acquire(&p->lock);
+
+      if (p->state == RUNNABLE)
+      {
+        if(!process || hp < p->priority)
+        {
+          if (process)
+            release(&process->lock);
+
+          process = p;
+          hp = p->priority;
+          continue;
+        }
+      }
+      release(&p->lock);
+    }
+
+    if (process)
+    {
+      process->state = RUNNING;
+      c->proc = process;
+      swtch(&c->context, &process->context);
+      c->proc = 0;
+      release(&process->lock);
+    }
  }
}
...
```

2. Πρόσθεσα ένα schedulertest user πρόγραμμα για να επιδείξω την λειτουργία του scheduler.

```diff
+# user/schedulertest
+#include "kernel/types.h"
+#include "kernel/stat.h"
+#include "user/user.h"
+#include "kernel/fcntl.h"
+
+
+#define NFORK 20
+#define IO 5
+
+int main() {
+  int n, pid;
+  for(n=0; n < NFORK;n++) {
+      pid = fork();
+      if (pid < 0)
+          break;
+      if (pid == 0) {
+            for (volatile int i = 0; i < 1000000000; i++) {} // CPU bound process 
+
+          printf("\n[+] Process:%d with Priority: %d finished", n, NFORK - n);
+          exit(0);
+      } else {
+        setpriority(NFORK - n, pid); // Will only matter for PBS, set lower priority for IO bound processes 
+      }
+  }
  
+  exit(0);
+}
```

3. Δεν ξέχασα την μαγεία στο Makefile

```diff
#  ./Makefile
+UPROGS=\
	$U/_cat\
 ...
+	$U/_schedulertest\
...
```

4. Αν τρέξει κανείς το πρόγραμμα εκτυπώνει κάτι τέτοιο:

```sh
$ schedulertest

[+] Process:0 wi
[t+] Ph Prirocoessr:ity1:  2wit0 hf iniPsrheidority: 19 finished
[+] Process:2 with Priority: 18 finished
[+] Process:5 with Priority: 15 finished
[+] Process:4 with Priority: 16 finished
[+] Process:3 with Priority: 17 finished
[+] Process:6 with Priority: 14 finished
[+] Process:8 with Priority: 12 finished$ 
[+] Process:7 with Priority: 13 finished
[+] Process:9 with Priority: 11 finished
[+] Process:10 with Priority: 10 finished
[+] Process:11 with Priority: 9 finished
[+] Process:12 with Priority: 8 finished
[+] Process:13 with Priority: 7 finished
[+] Process:14 with Priority: 6 finished
[+] Process:16 with Priority: 4 finished
[+] Process:15 with Priority: 5 finished
[+] Process:17 with Priority: 3 finished
[+] Process:18 with Priority: 2 finished
[+] Process:19 with Priority: 1 finished
```

## usertests

Εκτέλεσα τα `usertests` και όλα εντάξει ✅

