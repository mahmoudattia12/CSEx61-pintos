#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include "threads/synch.h"

// the only global lock, that we use to avoid race condition of files
static struct lock lock;

// A struct for the file descriptor entry
struct fd_entry
{
  int fd;
  struct file *file;
  struct list_elem elem;
};

static void syscall_handler(struct intr_frame *);
void check_valid_address(const void *ptr);
void halt(void);
void exit(int status);
bool create(char *file, unsigned initial_size);
bool remove(char *file);
tid_t exec(char *cmd_line);
int wait(int pid);
int open(char *file_name);
int fileSize(int fd);
int write(int fd, void *buffer, int size);
int read(int fd, void *buffer, int size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);
struct opened_file *get_file_by_fd(int fd);


void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&lock);
}


// function to validate that the address provided by the user is not null and inside my user space
// and it's mapped in the page table (have a physical address)
void check_valid_address(const void *pt)
{
  if (pt == NULL || !is_user_vaddr(pt) || pagedir_get_page(thread_current()->pagedir, pt) == NULL)
  {
    exit(-1);
  }
}


static void
syscall_handler(struct intr_frame *f)
{
  check_valid_address(f->esp);
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
    check_valid_address(esp + 4);
    int status = *((int *)esp + 1);
    exit(status);
    break;

  case SYS_EXEC:
    check_valid_address(esp + 4);
    char *cmd_line = (char *)(*((int *)esp + 1));
    if (cmd_line == NULL)
      exit(-1);
    lock_acquire(&lock);
    f->eax = exec(cmd_line);
    lock_release(&lock);
    break;

  case SYS_WAIT:
    check_valid_address(esp + 4);
    int pid = (*((int *)esp + 1));
    f->eax = wait(pid);
    break;

  case SYS_CREATE:
    check_valid_address(esp + 4);
    check_valid_address(esp + 8);
    file = (char *)(*((uint32_t *)esp + 1));
    unsigned init_size = *((unsigned *)esp + 2);
    if (file == NULL)
      exit(-1);
    f->eax = create(file, init_size);
    break;

  case SYS_REMOVE:
    check_valid_address(esp + 4);
    file = (char *)(*((uint32_t *)esp + 1));
    if (file == NULL)
      exit(-1);
    f->eax = remove(file);
    break;

  case SYS_OPEN:
    check_valid_address(esp + 4);
    char *file_name = (char *)(*((uint32_t *)esp + 1));
    if (file_name == NULL)
      exit(-1);
    f->eax = open(file_name);
    break;

  case SYS_FILESIZE:
    check_valid_address(esp + 4);
    fd = *((uint32_t *)esp + 1);
    f->eax = fileSize(fd);
    break;

  case SYS_READ:
    check_valid_address(esp + 4);
    check_valid_address(esp + 8);
    check_valid_address(esp + 12);

    fd = *((int *)f->esp + 1);
    buffer = (void *)(*((int *)f->esp + 2));
    size = *((int *)f->esp + 3);

    check_valid_address(buffer + size);

    f->eax = read(fd, buffer, size);
    break;

  case SYS_WRITE:
    check_valid_address(esp + 4);
    check_valid_address(esp + 8);
    check_valid_address(esp + 12);
    fd = *((uint32_t *)esp + 1);
    buffer = (void *)(*((uint32_t *)esp + 2));
    size = *((unsigned *)esp + 3);
    if (buffer == NULL)
      exit(-1);

    f->eax = write(fd, buffer, size);
    break;

  case SYS_SEEK:
    check_valid_address(esp + 4);
    check_valid_address(esp + 8);
    fd = *((uint32_t *)esp + 1);
    int pos = (*((unsigned *)esp + 2));
    seek(fd, pos);
    break;

  case SYS_TELL:
    check_valid_address(esp + 4);
    fd = *((uint32_t *)esp + 1);
    f->eax = tell(fd);
    break;

  case SYS_CLOSE:
    check_valid_address(esp + 4);
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


// system call for exiting myself and if i happened to have a parent then
// i'll set my parent's childStatus field to the status that i've terminated on
void exit(int status)
{
  struct thread *cur = thread_current()->parent;
  printf("%s: exit(%d)\n", thread_current()->name, status);
  if (cur)
    cur->childState = status;
  thread_exit();
}


// just call process_execute function and the process that will run this command line
// will be return
tid_t exec(char *cmd_line)
{
  return process_execute(cmd_line);
}

// Waits for a child process pid and retrieves the child's exit status.
// and this logic is done by process_wait function
int wait(int pid)
{
  return process_wait(pid);
}



// creating a file that's called (char *file) with (unsigned initial_size) as initial size
// but note that we must have the creation operation in a lock and finaly
// return boolean indicating whether the creation process is a success or not
bool create(char *file, unsigned initial_size)
{
  bool success;
	lock_acquire(&lock);
	success = filesys_create(file, initial_size);
	lock_release(&lock);
  return success;
}


// remove the file that's named (char *file) and return if success or not
bool remove(char *file)
{
  bool success;
  lock_acquire(&lock);
  success = filesys_remove(file);
  lock_release(&lock);
  return success;
}


int open(char *file_name)
{ 
  // allocate page in for file in memory
  struct opened_file *open = palloc_get_page(0);
  if (open == NULL)
  {
    palloc_free_page(open);
    return -1;
  }

  // open the file by filesys_open() and return a pointer to that file
  lock_acquire(&lock);
  open->ptr = filesys_open(file_name); //identify the file as a struct of file type
  lock_release(&lock);

  // if the pointer is null for any case such as no file name as such or any memory fail
  // then return -1 to indicate that we can't open that file
  if (open->ptr == NULL)
  {
    return -1;
  }
  // increment the directory to get the new index in the file directory table
  open->fileDescriptor = ++thread_current()->fileDirectory;
  // add to the current open files
  list_push_back(&thread_current()->file_list, &open->elem);
  return open->fileDescriptor;
}



int fileSize(int fileDescriptor)
{
  struct file *file = get_file_by_fd(fileDescriptor)->ptr;

  if (file == NULL)
  {
    // there's no file in my list with this descriptor
    return -1;
  }

  int fileLength;
  lock_acquire(&lock);
  // return the size of the opened file in bytes, and do not forget the lock
  fileLength = file_length(file);
  lock_release(&lock);
  return fileLength;
}

struct opened_file *get_file_by_fd(int fd)
{
  // takes the current fd as an argument, loop through the files possesed by the current thread
  // perform a linear search on any fileDesciptor that matches
  // if any return that file, else return NULL
  struct thread *t = thread_current();
  struct file *my_file = NULL;
  for (struct list_elem *e = list_begin(&t->file_list); e != list_end(&t->file_list);
       e = list_next(e))
  {
    struct opened_file *opened = list_entry(e, struct opened_file, elem);

    if (opened->fileDescriptor == fd)
    {
      return opened;
    }
  }
  return NULL;
}

/// changes the position of next byte to be written or read in the open file to "position" which is the offset
void seek(int fd, unsigned position)
{
  if (position < 0)
    return;
  struct file *fs = get_file_by_fd(fd)->ptr;
  if (fs == NULL)
    return;

  lock_acquire(&lock);
  file_seek(fs, position);
  lock_release(&lock);
}


/// get the pos of next file to be read or written in an open file
unsigned tell(int fd)
{
  struct file *f = get_file_by_fd(fd)->ptr;
  int pos;

  if (f == NULL)
    return -1;

  lock_acquire(&lock);
  // get the next byte to be read or written
  pos = (int)file_tell(f);
  lock_release(&lock);
  return pos;
}


void close(int fd)
{
  struct opened_file *f = get_file_by_fd(fd);

  if (f == NULL)
    return;

  lock_acquire(&lock);
  file_close(f->ptr);
  lock_release(&lock);

  // to release the resources so as not to get a memory leakage
  list_remove(&f->elem);
  palloc_free_page(f);
}


/// Writes (length) bytes from buffer to the open file fd.
int write(int fd, void *buffer, int length)
{
  int sizeActual = 0;
  struct thread *cur = thread_current();

  if (fd == 1)
  {
    // writing to console by the putbuf().
    lock_acquire(&lock);
    putbuf(buffer, length);
    sizeActual = (int)length; // the console is logically infinite so all the buffer is written to stdout
    lock_release(&lock);
  }
  //otherwise we have an opened file to write to, and we alrady have file_write to do that
  else
  { 
    // hold the file by the fd
    struct file *f = get_file_by_fd(fd)->ptr;
    lock_acquire(&lock);
    if (f == NULL)
      return -1; // no file
    sizeActual = (int)file_write(f, buffer, length);
    lock_release(&lock);
  }
  return sizeActual;
}

// read from file or from buffer
int read(int fd, void *buffer, int length)
{
  if (fd == 0)
  {//read from keyboard
    
    for (size_t i = 0; i < length; i++)
    {
      lock_acquire(&lock);// to check no write
      ((char*)buffer)[i] = input_getc();
      lock_release(&lock);
    }
    return length;
    
  } else {
// read from file
    struct thread* t = thread_current();
    struct file* f = get_file_by_fd(fd)->ptr;

    if (f == NULL)
    {
      return -1;//no opened file
    }

    int result;//the actual number of bytes be read
    lock_acquire(&lock);
    result = file_read(f,buffer,length);
    lock_release(&lock);
    return result;
  }
}