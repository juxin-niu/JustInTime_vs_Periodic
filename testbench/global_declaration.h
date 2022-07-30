
#ifndef TESTBENCH_GLOBAL_DECLARATION_H_
#define TESTBENCH_GLOBAL_DECLARATION_H_


#include <driver/driverlib_include.h>
#include <driver/led_button.h>
#include <driver/nv.h>
#include <driver/target.h>
#include <lib/math_lib/accel.h>
#include <lib/math_lib/mspmath/msp-math.h>

#include <stdint.h>
#include <stdbool.h>


// ====================================================  AR  ==================================================== //

#define AR_NUM_WARMUP_SAMPLES   3       // Number of samples to discard before recording training set
#define AR_ACCEL_WINDOW_SIZE    4
#define AR_MODEL_SIZE           5
#define AR_SAMPLE_NOISE_FLOOR   10      // Made up value
#define AR_SAMPLES_TO_COLLECT   20      // Number of classifications to complete in one experiment

typedef threeAxis_t_8 accelReading;
typedef accelReading accelWindow[AR_ACCEL_WINDOW_SIZE];

typedef struct {
    uint16_t meanmag;
    uint16_t stddevmag;
} ar_features_t;

typedef enum {
    CLASS_STATIONARY,
    CLASS_MOVING,
} ar_class_t;


typedef enum {
    MODE_IDLE = 3,
    MODE_TRAIN_STATIONARY = 2,
    MODE_TRAIN_MOVING = 1,
    MODE_RECOGNIZE = 0, // default
} ar_run_mode_t;

void AR_SingleSample(threeAxis_t_8* result, uint16_t* _v_seed);

// ====================================================  BC  ==================================================== //

#define BITCOUNT_SEED               4L
#define BITCOUNT_ITER               128
#define BITCOUNT_CHARBIT            8

extern const uint8_t bc_bits[256];

extern int Bitcount_RecursiveCnt(uint32_t x);
extern int Bitcount_NonRecursiveCnt(uint32_t x);

// ==================================================== CEM  ==================================================== //

#define CEM_NIL                         0 // like NULL, but for indexes, not real pointers

#define CEM_DICT_SIZE                   192
#define CEM_BLOCK_SIZE                  96

#define CEM_NUM_LETTERS_IN_SAMPLE       2
#define CEM_LETTER_MASK                 0x000F
#define CEM_LETTER_SIZE_BITS            8
#define CEM_NUM_LETTERS                 (CEM_LETTER_MASK + 1)

typedef uint16_t cem_index_t;
typedef uint16_t cem_letter_t;
typedef uint16_t cem_sample_t;

// NOTE: can't use pointers, since need to ChSync, etc
typedef struct {
    cem_letter_t letter; // 'letter' of the alphabet
    cem_index_t sibling; // this node is a member of the parent's children list
    cem_index_t child;   // link-list of children
} cem_node_t;

extern cem_sample_t CEM_AcquireSample(cem_letter_t prev_sample);

// ==================================================== CRC  ==================================================== //

#define CRC_LENGTH          (1024)
#define CRC_INIT            (0xFFFF)

extern const uint16_t CRC_Input[CRC_LENGTH];

extern uint16_t CRCheck_CCITT_Update(uint16_t init, uint16_t input);

// =================================================== CUCKOO  ================================================== //

#define CUCKOO_NUM_BUCKETS          256                             // must be a power of 2
#define CUCKOO_NUM_INSERTS          (CUCKOO_NUM_BUCKETS / 2)        // shoot for 25% occupancy
#define CUCKOO_NUM_LOOKUPS          CUCKOO_NUM_INSERTS
#define CUCKOO_MAX_RELOCATIONS      8
#define CUCKOO_BUFFER_SIZE          128

typedef uint16_t cuckoo_value_t;
typedef uint16_t cuckoo_hash_t;
typedef uint16_t cuckoo_fingerprint_t;
typedef uint16_t cuckoo_index_t;               // bucket index

typedef struct {
    uint16_t insert_count;
    uint16_t inserted_count;
} cuckoo_insert_count_t;

typedef struct {
    uint16_t lookup_count;
    uint16_t member_count;
} cuckoo_lookup_count_t;


typedef enum{
    CUCKOO_Insert,
    CUCKOO_Add,
    CUCKOO_Lookup,
    CUCKOO_Lookup_Search
} TaskName;

// seeds the pseudo-random sequence of keys
extern const cuckoo_value_t cuckoo_init_key;

extern cuckoo_index_t CUCKOO_Hash2Index(cuckoo_fingerprint_t fp);

extern cuckoo_fingerprint_t CUCKOO_Hash2Fingerprint(cuckoo_value_t key);

// ================================================== DIJKSTRA  ================================================== //

typedef struct {
    uint16_t dist;
    uint16_t prev;
} dijkstra_node_t;

typedef struct {
    uint16_t node;
    uint16_t dist;
    uint16_t prev;
} dijkstra_queue_t;

#define DIJKSTRA_NNODES             25
#define DIJKSTRA_QSIZE              4 * DIJKSTRA_NNODES
#define DIJKSTRA_INFINITY           0xFFFF
#define DIJKSTRA_UNDEFINED          0xFFFF


extern const uint8_t adj_matrix[DIJKSTRA_NNODES][DIJKSTRA_NNODES];

// ==================================================== RSA  ===================================================== //

extern const uint16_t rsa_msg[38];

extern const uint16_t p;
extern const uint16_t q;

#define RSA_MSGLENGTH    (30)

extern uint16_t RSA_PowerMod(uint16_t a, uint16_t b, uint16_t mod);
extern uint16_t RSA_Sqrt16(uint16_t x);
extern uint16_t RSA_ModInv(uint16_t u, uint16_t v);
extern uint8_t RSA_isPrime(uint16_t x, uint16_t sqrt_x);

// ==================================================== SORT  ===================================================== //

#define SORT_LENGTH  100

extern const uint16_t raw[SORT_LENGTH];



#endif /* TESTBENCH_GLOBAL_DECLARATION_H_ */
