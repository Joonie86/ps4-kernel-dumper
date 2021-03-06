#include "kernel_utils.h"

#define KERNEL_CHUNK_SIZE 0x1000
#define KERNEL_CHUNK_NUMBER 0x69B8

int _main(struct thread* td) {
  initKernel();
  initLibc();
  initPthread();

  uint64_t fw_version = get_fw_version();
  jailbreak(fw_version);
  uint64_t kernel_base = get_kernel_base(fw_version);

  initSysUtil();

  printf_notification("Running PS4 Kernel Dumper");
  char saveFile[64];
  int sf = -1;
  int row = 0;
  while (sf == -1) {
    sceKernelUsleep(100 * 1000);
    if (row >= 60) {
      printf_notification("No USB storage device detected.\nPlease connect one.");
      row = 0;
    } else {
      row += 1;
    }
    sprintf(saveFile, "/mnt/usb%i/kernel_dump_fw_%03X.bin", row / 10, fw_version);
    sf = open(saveFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
  }
  printf_notification("\nUSB device detected.\n\nStarting kernel dumping to USB%i.", row / 10);
  int percent = 0;

  uint64_t* dump = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  uint64_t pos = 0;
  for (uint64_t i = 0; i < KERNEL_CHUNK_NUMBER; i++) {
    dump_kernel(fw_version, kernel_base + pos, dump, KERNEL_CHUNK_SIZE);
    lseek(sf, pos, SEEK_SET);
    write(sf, (void*)dump, KERNEL_CHUNK_SIZE);
    if (i >= (percent + 10) * KERNEL_CHUNK_NUMBER / 100) {
      percent += 10;
      printf_notification("Kernel dumping to USB%i\nDone: %i%%", row / 10, percent);
    }
    pos = pos + KERNEL_CHUNK_SIZE;
  }
  printf_notification("Kernel dumped successfully!");

  close(sf);

  munmap(dump, 0x1000);

  return 0;
}
