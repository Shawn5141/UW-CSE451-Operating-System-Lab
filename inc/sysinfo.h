#pragma once

struct sys_info {
  int pages_in_use;
  int pages_in_swap;
  int free_pages;
  int num_page_faults;
  int num_disk_reads;
};
