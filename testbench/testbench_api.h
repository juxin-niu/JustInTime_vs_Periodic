

#ifndef APPS_APP_H_
#define APPS_APP_H_

void jit_ar_main();
void jit_bc_main();
void jit_cem_main();
void jit_crc_main();
void jit_cuckoo_main();
void jit_dijkstra_main();
void jit_rsa_main();
void jit_sort_main();

void pc_ar_main();
void pc_bc_main();
void pc_cem_main();
void pc_crc_main();
void pc_cuckoo_main();
void pc_dijkstra_main();
void pc_rsa_main();
void pc_sort_main();

typedef void (*FUNCPTR)();

#endif /* APPS_APP_H_ */
