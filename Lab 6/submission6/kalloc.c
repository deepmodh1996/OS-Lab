// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h" 

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  int num_free_pages;   ////==  Try something
  int max;
  int reference_count[PHYSTOP/PGSIZE];
  struct run *freelist;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

int
refer_count(uint pa, int value)
{
  int val = 0;
  if (pa%PGSIZE != 0)
    panic("Physical address is not alligned");
  if(kmem.use_lock)
    acquire(&kmem.lock);
  kmem.reference_count[pa/PGSIZE] += value;
  val = kmem.reference_count[pa/PGSIZE];
  // cprintf("refercount --- %d, %d\n", pa/PGSIZE, value);
  if(kmem.use_lock)
    release(&kmem.lock);
  return val;
}

//PAGEBREAK: 21
/*uint
myV2P (pde_t *pgdir, void *va)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {

  }
  return PTE_ADDR(pgtab[PTX(va)]);
}*/

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  if(kmem.use_lock)
    acquire(&kmem.lock);
  if (kmem.reference_count[V2P(v)/PGSIZE] > 1) {
    kmem.reference_count[V2P(v)/PGSIZE]--;
    // cprintf("Kfree --- %d\n", V2P(v)/PGSIZE);
  } else {
    // Fill with junk to catch dangling refs.
    memset(v, 1, PGSIZE);

    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
    kmem.reference_count[V2P(v)/PGSIZE] = 0;
    // cprintf("Kfree 0 %d\n", V2P(v)/PGSIZE);
    kmem.num_free_pages++;    ////== increase when added to list
    if (kmem.num_free_pages > kmem.max) {
      kmem.max = kmem.num_free_pages;
    }
  }
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.num_free_pages--;    ////== decreases when removed from list
    // char *temp = (char*)r;      ////== ohoho
    // uint pa = V2P((char*)r);
    kmem.reference_count[V2P((char*)r)/PGSIZE] = 1;
    // cprintf("Kalloc %d\n", V2P((char*)r)/PGSIZE);
  }
  if(kmem.use_lock)
    release(&kmem.lock);

  return (char*)r;
}

int
kfreepages(void)
{
  return kmem.num_free_pages;   ////== function to get the number of free pages
}