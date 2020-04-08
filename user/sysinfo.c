#include <cdefs.h>
#include <fs.h>
#include <stat.h>
#include <sysinfo.h>
#include <user.h>

int main(int argc, char *argv[]) {
  struct sys_info info;
  sysinfo(&info);

  printf(1, "pages_in_use = %d\n", info.pages_in_use);
  printf(1, "pages_in_swap = %d\n", info.pages_in_swap);
  printf(1, "free_pages = %d\n", info.free_pages);
  printf(1, "num_page_faults = %d\n", info.num_page_faults);
  printf(1, "num_disk_reads = %d\n", info.num_disk_reads);

  exit();
}
