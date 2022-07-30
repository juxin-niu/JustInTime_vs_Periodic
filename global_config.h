

#ifndef GLOBAL_CONFIG_H_
#define GLOBAL_CONFIG_H_

// ===================== MAGIC NUMBER ====================== //

#define BACKUP_WITH_DMA     0x01
#define BACKUP_WITH_CPU     0x00

// ===================== GLOBAL CONFIG ===================== //


#define PC_BACKUP_MODE      BACKUP_WITH_DMA
#define JIT_BACKUP_MODE     BACKUP_WITH_DMA

#define VOLT_MONITOR_INTERVAL       200

#define RECOVERY_VOLT_THRESHOLD     3000
#define BACKUP_VOLT_THRESHOLD       2000

#define SEND_SINGNAL_AT_START_AND_END   true

#endif /* GLOBAL_CONFIG_H_ */
