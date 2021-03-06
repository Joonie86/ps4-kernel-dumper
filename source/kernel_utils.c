#include "kernel_utils.h"

#define X86_CR0_WP (1 << 16)

static inline __attribute__((always_inline)) uint64_t __readmsr(unsigned long __register) {
  unsigned long __edx;
  unsigned long __eax;
  __asm__("rdmsr"
          : "=d"(__edx), "=a"(__eax)
          : "c"(__register));
  return (((uint64_t)__edx) << 32) | (uint64_t)__eax;
}

static inline __attribute__((always_inline)) uint64_t readCr0(void) {
  uint64_t cr0;
  __asm__ volatile("movq %0, %%cr0"
                   : "=r"(cr0)
                   :
                   : "memory");
  return cr0;
}

static inline __attribute__((always_inline)) void writeCr0(uint64_t cr0) {
  __asm__ volatile("movq %%cr0, %0"
                   :
                   : "r"(cr0)
                   : "memory");
}

int kpayload_get_fw_version(struct thread *td, struct kpayload_get_fw_version_args *args) {
  void *kernel_base = 0;
  int (*copyout)(const void *kaddr, void *uaddr, size_t len) = 0;

  uint64_t fw_version = 0x666;

  if (!memcmp((char *)(&((uint8_t *)__readmsr(0xC0000082))[-K672_XFAST_SYSCALL]), (char[4]){0x7F, 0x45, 0x4C, 0x46}, 4)) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K672_XFAST_SYSCALL];
    if (!memcmp((char *)(kernel_base + K672_PRINTF), (char[12]){0x55, 0x48, 0x89, 0xE5, 0x53, 0x48, 0x83, 0xEC, 0x58, 0x48, 0x8D, 0x1D}, 12)) {
      fw_version = 0x672;
      copyout = (void *)(kernel_base + K672_COPYOUT);
    } else if (!memcmp((char *)(kernel_base + K505_PRINTF), (char[12]){0x55, 0x48, 0x89, 0xE5, 0x53, 0x48, 0x83, 0xEC, 0x58, 0x48, 0x8D, 0x1D}, 12)) {
      fw_version = 0x505;
      copyout = (void *)(kernel_base + K505_COPYOUT);
    } else if (!memcmp((char *)(kernel_base + K503_PRINTF), (char[12]){0x55, 0x48, 0x89, 0xE5, 0x53, 0x48, 0x83, 0xEC, 0x58, 0x48, 0x8D, 0x1D}, 12)) {
      fw_version = 0x503;
      copyout = (void *)(kernel_base + K503_COPYOUT);
    } else if (!memcmp((char *)(kernel_base + K501_PRINTF), (char[12]){0x55, 0x48, 0x89, 0xE5, 0x53, 0x48, 0x83, 0xEC, 0x58, 0x48, 0x8D, 0x1D}, 12)) {
      fw_version = 0x501;
      copyout = (void *)(kernel_base + K501_COPYOUT);
    }
  } else if (!memcmp((char *)(&((uint8_t *)__readmsr(0xC0000082))[-K474_XFAST_SYSCALL]), (char[4]){0x7F, 0x45, 0x4C, 0x46}, 4)) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K474_XFAST_SYSCALL];
    if (!memcmp((char *)(kernel_base + K474_PRINTF), (char[12]){0x55, 0x48, 0x89, 0xE5, 0x53, 0x48, 0x83, 0xEC, 0x58, 0x48, 0x8D, 0x1D}, 12)) {
      fw_version = 0x474;
      copyout = (void *)(kernel_base + K474_COPYOUT);
    }
  } else if (!memcmp((char *)(&((uint8_t *)__readmsr(0xC0000082))[-K455_XFAST_SYSCALL]), (char[4]){0x7F, 0x45, 0x4C, 0x46}, 4)) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K455_XFAST_SYSCALL];
    if (!memcmp((char *)(kernel_base + K455_PRINTF), (char[12]){0x55, 0x48, 0x89, 0xE5, 0x53, 0x48, 0x83, 0xEC, 0x58, 0x48, 0x8D, 0x1D}, 12)) {
      fw_version = 0x455;
      copyout = (void *)(kernel_base + K455_COPYOUT);
    }
  } else if (!memcmp((char *)(&((uint8_t *)__readmsr(0xC0000082))[-K405_XFAST_SYSCALL]), (char[4]){0x7F, 0x45, 0x4C, 0x46}, 4)) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K405_XFAST_SYSCALL];
    if (!memcmp((char *)(kernel_base + K405_PRINTF), (char[12]){0x55, 0x48, 0x89, 0xE5, 0x53, 0x48, 0x83, 0xEC, 0x58, 0x48, 0x8D, 0x1D}, 12)) {
      fw_version = 0x405;
      copyout = (void *)(kernel_base + K405_COPYOUT);
    }
  } else {
    return -1;
  }

  uint64_t uaddr = args->kpayload_get_fw_version_info->uaddr;
  copyout(&fw_version, (uint64_t *)uaddr, 8);

  return 0;
}

int kpayload_jailbreak(struct thread *td, struct kpayload_jailbreak_args *args) {
  struct filedesc *fd;
  struct ucred *cred;
  fd = td->td_proc->p_fd;
  cred = td->td_proc->p_ucred;

  void *kernel_base;
  uint8_t *kernel_ptr;
  void **got_prison0;
  void **got_rootvnode;

  uint64_t fw_version = args->kpayload_jailbreak_info->fw_version;

  if (fw_version == 0x405) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K405_XFAST_SYSCALL];
    kernel_ptr = (uint8_t *)kernel_base;
    got_prison0 = (void **)&kernel_ptr[K405_PRISON_0];
    got_rootvnode = (void **)&kernel_ptr[K405_ROOTVNODE];
  } else if (fw_version == 0x455) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K455_XFAST_SYSCALL];
    kernel_ptr = (uint8_t *)kernel_base;
    got_prison0 = (void **)&kernel_ptr[K455_PRISON_0];
    got_rootvnode = (void **)&kernel_ptr[K455_ROOTVNODE];
  } else if (fw_version == 0x474) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K474_XFAST_SYSCALL];
    kernel_ptr = (uint8_t *)kernel_base;
    got_prison0 = (void **)&kernel_ptr[K474_PRISON_0];
    got_rootvnode = (void **)&kernel_ptr[K474_ROOTVNODE];
  } else if (fw_version == 0x501) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K501_XFAST_SYSCALL];
    kernel_ptr = (uint8_t *)kernel_base;
    got_prison0 = (void **)&kernel_ptr[K501_PRISON_0];
    got_rootvnode = (void **)&kernel_ptr[K501_ROOTVNODE];
  } else if (fw_version == 0x503) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K503_XFAST_SYSCALL];
    kernel_ptr = (uint8_t *)kernel_base;
    got_prison0 = (void **)&kernel_ptr[K503_PRISON_0];
    got_rootvnode = (void **)&kernel_ptr[K503_ROOTVNODE];
  } else if (fw_version == 0x505) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K505_XFAST_SYSCALL];
    kernel_ptr = (uint8_t *)kernel_base;
    got_prison0 = (void **)&kernel_ptr[K505_PRISON_0];
    got_rootvnode = (void **)&kernel_ptr[K505_ROOTVNODE];
  } else if (fw_version == 0x672) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K672_XFAST_SYSCALL];
    kernel_ptr = (uint8_t *)kernel_base;
    got_prison0 = (void **)&kernel_ptr[K672_PRISON_0];
    got_rootvnode = (void **)&kernel_ptr[K672_ROOTVNODE];
  } else {
    return -1;
  }

  cred->cr_uid = 0;
  cred->cr_ruid = 0;
  cred->cr_rgid = 0;
  cred->cr_groups[0] = 0;

  cred->cr_prison = *got_prison0;
  fd->fd_rdir = fd->fd_jdir = *got_rootvnode;

  void *td_ucred = *(void **)(((char *)td) + 304);

  uint64_t *sonyCred = (uint64_t *)(((char *)td_ucred) + 96);
  *sonyCred = 0xffffffffffffffff;

  uint64_t *sceProcessAuthorityId = (uint64_t *)(((char *)td_ucred) + 88);
  *sceProcessAuthorityId = 0x3801000000000013;

  uint64_t *sceProcCap = (uint64_t *)(((char *)td_ucred) + 104);
  *sceProcCap = 0xffffffffffffffff;

  return 0;
}

int kpayload_get_kbase(struct thread *td, struct kpayload_get_kbase_args *args) {
  void *kernel_base;
  int (*copyout)(const void *kaddr, void *uaddr, size_t len);

  uint64_t fw_version = args->kpayload_get_kbase_info->fw_version;

  if (fw_version == 0x405) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K405_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K405_COPYOUT);
  } else if (fw_version == 0x455) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K455_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K455_COPYOUT);
  } else if (fw_version == 0x474) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K474_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K474_COPYOUT);
  } else if (fw_version == 0x501) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K501_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K501_COPYOUT);
  } else if (fw_version == 0x503) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K503_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K503_COPYOUT);
  } else if (fw_version == 0x505) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K505_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K505_COPYOUT);
  } else if (fw_version == 0x672) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K672_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K672_COPYOUT);
  } else {
    return -1;
  }

  uint64_t uaddr = args->kpayload_get_kbase_info->uaddr;
  copyout(&kernel_base, (uint64_t *)uaddr, 8);

  return 0;
}

int kpayload_kernel_dumper(struct thread *td, struct kpayload_kernel_dumper_args *args) {
  void *kernel_base;
  int (*copyout)(const void *kaddr, void *uaddr, size_t len);

  uint64_t fw_version = args->kpayload_kernel_dumper_info->fw_version;

  if (fw_version == 0x405) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K405_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K405_COPYOUT);
  } else if (fw_version == 0x455) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K455_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K455_COPYOUT);
  } else if (fw_version == 0x474) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K474_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K474_COPYOUT);
  } else if (fw_version == 0x501) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K501_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K501_COPYOUT);
  } else if (fw_version == 0x503) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K503_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K503_COPYOUT);
  } else if (fw_version == 0x505) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K505_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K505_COPYOUT);
  } else if (fw_version == 0x672) {
    kernel_base = &((uint8_t *)__readmsr(0xC0000082))[-K672_XFAST_SYSCALL];
    copyout = (void *)(kernel_base + K672_COPYOUT);
  } else {
    return -1;
  }

  uint64_t kaddr = args->kpayload_kernel_dumper_info->kaddr;
  uint64_t uaddr = args->kpayload_kernel_dumper_info->uaddr;
  size_t size = args->kpayload_kernel_dumper_info->size;

  int cpRet = copyout((uint64_t *)kaddr, (uint64_t *)uaddr, size);

  if (cpRet == -1) {
    memset((uint64_t *)uaddr, 0, size);
  }

  return cpRet;
}

uint64_t get_fw_version(void) {
  uint64_t fw_version = 0x666;
  uint64_t *fw_version_ptr = mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  struct kpayload_get_fw_version_info kpayload_get_fw_version_info;
  kpayload_get_fw_version_info.uaddr = (uint64_t)fw_version_ptr;
  kexec(&kpayload_get_fw_version, &kpayload_get_fw_version_info);
  memcpy(&fw_version, fw_version_ptr, 8);
  munmap(fw_version_ptr, 8);

  return fw_version;
}

int jailbreak(uint64_t fw_version) {
  struct kpayload_jailbreak_info kpayload_jailbreak_info;
  kpayload_jailbreak_info.fw_version = fw_version;
  kexec(&kpayload_jailbreak, &kpayload_jailbreak_info);

  return 0;
}

uint64_t get_kernel_base(uint64_t fw_version) {
  uint64_t kernel_base = -1;
  uint64_t *kernel_base_ptr = mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  struct kpayload_get_kbase_info kpayload_get_kbase_info;
  kpayload_get_kbase_info.fw_version = fw_version;
  kpayload_get_kbase_info.uaddr = (uint64_t)kernel_base_ptr;
  kexec(&kpayload_get_kbase, &kpayload_get_kbase_info);
  memcpy(&kernel_base, kernel_base_ptr, 8);
  munmap(kernel_base_ptr, 8);

  return kernel_base;
}

int dump_kernel(uint64_t fw_version, uint64_t kaddr, uint64_t *dump, size_t size) {
  struct kpayload_kernel_dumper_info kpayload_kernel_dumper_info;
  kpayload_kernel_dumper_info.fw_version = fw_version;
  kpayload_kernel_dumper_info.kaddr = kaddr;
  kpayload_kernel_dumper_info.uaddr = (uint64_t)dump;
  kpayload_kernel_dumper_info.size = size;
  kexec(&kpayload_kernel_dumper, &kpayload_kernel_dumper_info);

  return 0;
}
