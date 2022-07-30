
#include <testbench/global_declaration.h>

void AR_SingleSample(threeAxis_t_8* result, uint16_t* _v_seed)
{
    unsigned seed = *_v_seed;

    result->x = (seed * 17) % 85;
    result->y = (seed * 17 * 17) % 85;
    result->z = (seed * 17 * 17 * 17) % 85;
    *_v_seed = seed + 1;
}

int Bitcount_RecursiveCnt(uint32_t x)
{
    int cnt = bc_bits[(int)(x & 0x0000000FL)];

    if (0L != (x >>= 4))
        cnt += Bitcount_RecursiveCnt(x);

    return cnt;
}

int Bitcount_NonRecursiveCnt(uint32_t x)
{
    int cnt = bc_bits[(int)(x & 0x0000000FL)];

    while (0L != (x >>= 4)) {
        cnt += bc_bits[(int)(x & 0x0000000FL)];
    }

    return cnt;
}

cem_sample_t CEM_AcquireSample(cem_letter_t prev_sample)
{
    cem_letter_t sample = (prev_sample + 1) & 0x03;
    return sample;
}

uint16_t CRCheck_CCITT_Update(uint16_t init, uint16_t input)
{
    uint16_t CCITT = (uint8_t) (init >> 8) | (init << 8);
    CCITT ^= input;
    CCITT ^= (uint8_t) (CCITT & 0xFF) >> 4;
    CCITT ^= (CCITT << 8) << 4;
    CCITT ^= ((CCITT & 0xFF) << 4) << 1;
    return CCITT;
}

cuckoo_hash_t CUCKOO_DjbHash(uint8_t* data, uint16_t len)
{
    uint16_t hash = 5381;
    uint16_t i;

    for (i = 0; i < len; data++, i++)
        hash = ((hash << 5) + hash) + (*data);

    return hash & 0xFFFF;
}

cuckoo_index_t CUCKOO_Hash2Index(cuckoo_fingerprint_t fp)
{
    cuckoo_hash_t hash = CUCKOO_DjbHash((uint8_t *) &fp, sizeof(cuckoo_fingerprint_t));
    return hash & (CUCKOO_NUM_BUCKETS - 1); // NUM_BUCKETS must be power of 2
}

cuckoo_fingerprint_t CUCKOO_Hash2Fingerprint(cuckoo_value_t key)
{
    return CUCKOO_DjbHash((uint8_t *) &key, sizeof(cuckoo_value_t));
}

uint16_t RSA_PowerMod(uint16_t a, uint16_t b, uint16_t mod)
{
    uint16_t ans = 1;
    a = a % mod;
    while (b > 0) {
        if (b % 2 == 1)
            ans = (ans * a) % mod;
        b = b / 2;
        a = (a * a) % mod;
    }
    return ans;
}

/* Square root by Newton's method. This code is from InK */
uint16_t RSA_Sqrt16(uint16_t x)
{
    uint16_t hi = 0xffff;
    uint16_t lo = 0;
    uint16_t mid = ((uint32_t)hi + (uint32_t)lo) >> 1;
    uint32_t s = 0;

    while (s != x && hi - lo > 1) {
        mid = ((uint32_t)hi + (uint32_t)lo) >> 1;
        s = (uint32_t)mid* (uint32_t)mid;
        if (s < x)
            lo = mid;
        else
            hi = mid;
    }

    return mid;
}


/* https://www.di-mgt.com.au/euclidean.html#extendedeuclidean */
uint16_t RSA_ModInv(uint16_t u, uint16_t v)
{
    uint16_t inv, u1, u3, v1, v3, t1, t3, q;
    int16_t iter;
    u1 = 1;
    u3 = u;
    v1 = 0;
    v3 = v;
    iter = 1;
    while (v3 != 0)
    {
        q = u3 / v3;
        t3 = u3 % v3;
        t1 = u1 + q * v1;
        u1 = v1; v1 = t1; u3 = v3; v3 = t3;
        iter = -iter;
    }
    if (iter < 0)
        inv = v - u1;
    else
        inv = u1;
    return inv;
}

uint8_t RSA_isPrime(uint16_t x, uint16_t sqrt_x)
{
    uint16_t i;
    for (i = 3; i <= sqrt_x; i += 2)
    {
        if (x % i == 0)
            return 0;
    }
    return 1;
}

