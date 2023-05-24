#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

//the only global lock, that we use to provide mutual execlusion over the files
static struct lock lock;


//A struct for the file descriptor entry
struct fd_entry
{
	int fd;
	struct file *file;
	struct list_elem elem;
};


static void syscall_handler(struct intr_frame *);
void validate_void_ptr(const void *ptr);
void halt(void);
void exit(int status);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&lock);
}

// function to validate that the address provided by the user is not null and inside my user space
// and it's mapped in the page table (have a physical address)
void validate_void_ptr(const void *pt)
{
  if (pt == NULL || !is_user_vaddr(pt) || pagedir_get_page(thread_current()->pagedir, pt) == NULL)
  {
    exit(-1);
  }
}


static void
syscall_handler(struct intr_frame *f UNUSED)
{
  validate_void_ptr(f->esp);
  void *esp = f->esp;
  int fd;
  void *buffer;
  int size;
  char *file;
  switch (*(int *)esp)
  {
  case SYS_HALT:
    halt();
    break;

  case SYS_EXIT:
    validate_void_ptr(esp + 4);
    int status = *((int *)esp + 1);
    exit(status);
    break;

  case SYS_EXEC:
    validate_void_ptr(esp + 4);
    char *cmd_line = (char *)(*((int *)esp + 1));
    if (cmd_line == NULL)
      exit(-1);
    lock_acquire(&lock);
    f->eax = sys_exec(cmd_line);
    lock_release(&lock);
    break;

  case SYS_WAIT:
    validate_void_ptr(esp + 4);
    int pid = (*((int *)esp + 1));
    f->eax = wait(pid);
    break;

  case SYS_CREATE:
    validate_void_ptr(esp + 4);
    validate_void_ptr(esp + 8);
    file = (char *)(*((uint32_t *)esp + 1));
    unsigned init_size = *((unsigned *)esp + 2);
    if (file == NULL)
      exit(-1);
    f->eax = create(file, init_size);
    break;

  case SYS_REMOVE:
    validate_void_ptr(esp + 4);
    file = (char *)(*((uint32_t *)esp + 1));
    if (file == NULL)
      exit(-1);
    f->eax = remove(file);
    break;

  case SYS_OPEN:
    validate_void_ptr(esp + 4);
    char *file_name = (char *)(*((uint32_t *)esp + 1));
    if (file_name == NULL)
      exit(-1);
    f->eax = open(file_name);
    break;

  case SYS_FILESIZE:
    validate_void_ptr(esp + 4);
    fd = *((uint32_t *)esp + 1);
    f->eax = file_size(fd);
    break;

  case SYS_READ:
    validate_void_ptr(esp + 4);
    validate_void_ptr(esp + 8);
    validate_void_ptr(esp + 12);

    fd = *((int *)f->esp + 1);
    buffer = (void *)(*((int *)f->esp + 2));
    size = *((int *)f->esp + 3);

    validate_void_ptr(buffer + size);

    f->eax = read(fd, buffer, size);
    break;

  case SYS_WRITE:
    validate_void_ptr(esp + 4);
    validate_void_ptr(esp + 8);
    validate_void_ptr(esp + 12);
    fd = *((uint32_t *)esp + 1);
    buffer = (void *)(*((uint32_t *)esp + 2));
    size = *((unsigned *)esp + 3);
    if (buffer == NULL)
      exit(-1);

    f->eax = write(fd, buffer, size);
    break;

  case SYS_SEEK:
    validate_void_ptr(esp + 4);
    validate_void_ptr(esp + 8);
    fd = *((uint32_t *)esp + 1);
    int pos = (*((unsigned *)esp + 2));
    seek(fd, pos);
    break;

  case SYS_TELL:
    validate_void_ptr(esp + 4);
    fd = *((uint32_t *)esp + 1);
    f->eax = tell(fd);
    break;

  case SYS_CLOSE:
    validate_void_ptr(esp + 4);
    fd = *((uint32_t *)esp + 1);
    close(fd);
    break;
  default:
    break;
  }
}



// System call to shut down the system and halt the OS
void halt(void)
{
  shutdown_power_off();
}


//system call for exiting myself and if i happened to have a parent then 
//i'll set my parent's childStatus field to the status that i've terminated on
void exit(int status)
{
  struct thread *cur = thread_current()->parent;
  printf("%s: exit(%d)\n", thread_current()->name, status);
  if (cur)
    cur->childStatus = status;
  thread_exit();
}


//just call process_execute function and the process that will run this command line
//will be return 
tid_t exec(char *cmd_line)
{
	return process_execute(cmd_line);
}



//Waits for a child process pid and retrieves the child's exit status.
//and this logic is done by process_wait function 
int wait(int pid)
{
	return process_wait(pid);
}


//creating a file that's called (char *file) with (unsigned initial_size) as initial size
//but note that we must have the creation operation in a lock and finaly 
//return boolean indicating whether the creation process is a success or not
bool create(char *file, unsigned initial_size)
{
  bool ret;
	lock_acquire(&lock);
	ret = filesys_create(file, initial_size);
	lock_release(&lock);
	return ret;
}

//remove the file that's named (char *file) and return if success or not
bool remove(char *file)
{
  bool ret;
	lock_acquire(&lock);
	ret = filesys_remove(file);
	lock_release(&lock);
	return ret;
}





