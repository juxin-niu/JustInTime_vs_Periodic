
#include <testbench/global_declaration.h>
#include <testbench/Periodic_scheduler.h>
#include <testbench/testbench_api.h>

__SHARED_VAR(
uint16_t        i_debug;
uint16_t        ar_v_seed;

uint16_t        _v_count;
uint16_t        _v_pinState;
uint16_t        _v_samplesInWindow;
uint16_t        _v_totalCount;
uint16_t        _v_movingCount;
uint16_t        _v_stationaryCount;
uint16_t        _v_discardedSamplesCount;
uint16_t        _v_trainingSetSize;

ar_class_t      _v_class;
ar_run_mode_t   _v_mode;
ar_features_t   _v_features;
uint16_t        _v_meanmag;
uint16_t        _v_stddevmag;
accelReading    _v_window[AR_ACCEL_WINDOW_SIZE];
ar_features_t   _v_model_stationary[AR_MODEL_SIZE];
ar_features_t   _v_model_moving[AR_MODEL_SIZE];

uint16_t        resultStationaryPct[4];
uint16_t        resultMovingPct[4];
uint16_t        sum[4];
)


static __nv bool      first_run = 1;
static __nv uint16_t  status = 0;
static const uint16_t global_war_size = 10;
static __nv uint16_t  backup_buf[10] = {};

static const bool backup_needed[] = {
    false, true, true, false, false, false, true, true, true
};

void pc_ar_main()
{
    // Local variables

    uint16_t        i;
    uint16_t        pin_state;
    accelReading    aR_sample;
    accelReading    mean;
    accelReading    stddev;
    ar_features_t   features;
    uint16_t        move_less_error;
    uint16_t        stat_less_error;
    uint32_t        l_meanmag;
    uint32_t        l_stddevmag;
    ar_features_t   ms;
    ar_features_t   mm;
    int32_t         stat_mean_err;
    int32_t         stat_sd_err;
    int32_t         move_mean_err;
    int32_t         move_sd_err;
    threeAxis_t_8   tA8_sample;

    // Build-in scheduler


    if (first_run == 1) { status = 0; first_run = 0;}
    else {
        if (__IS_TASK_RUNNING && backup_needed[__GET_CURTASK] == true) {
            BUILDIN_UNDO;
        }
    }

    PREPARE_FOR_BACKUP;

    switch(__GET_CURTASK) {
    case 0: goto init;
    case 1: goto Select_Mode;
    case 2: goto AR_Sample;
    case 3: goto AR_Transform;
    case 4: goto AR_Featurize;
    case 5: goto AR_Classify;
    case 6: goto AR_Stats;
    case 7: goto Warm_Up;
    case 8: goto AR_Train;
    }

    // Tasks
    // =================================================================
    __BUILDIN_TASK_BOUNDARY(0, init);
    __GET(i_debug) = 0;
    __GET(ar_v_seed) = 0x1111;
    __GET(_v_pinState) = MODE_IDLE;
    __GET(_v_count) = 0;
    __NEXT(1, Select_Mode);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(1, Select_Mode);
    pin_state = MODE_TRAIN_MOVING;  // 1
    __GET(_v_count)++;

    if(__GET(_v_count) >= 7)            { __FINISH; }
    else if(__GET(_v_count) >= 3)       pin_state = MODE_RECOGNIZE;        // 0
    else if(__GET(_v_count) >= 2)       pin_state = MODE_TRAIN_STATIONARY; // 2

    if ( (pin_state == MODE_TRAIN_STATIONARY || pin_state == MODE_TRAIN_MOVING)
         && (pin_state == __GET(_v_pinState)) ) {
     pin_state = MODE_IDLE;
    }
    else {
     __GET(_v_pinState) = pin_state;
    }

    switch(pin_state)
    {
    case MODE_TRAIN_STATIONARY:
         __GET(_v_discardedSamplesCount) = 0;
         __GET(_v_mode) = MODE_TRAIN_STATIONARY;
         __GET(_v_class) = CLASS_STATIONARY;
         __GET(_v_samplesInWindow) = 0;
         __NEXT(7, Warm_Up);

    case MODE_TRAIN_MOVING:
         __GET(_v_discardedSamplesCount) = 0;
         __GET(_v_mode) = MODE_TRAIN_MOVING;
         __GET(_v_class) = CLASS_MOVING;
         __GET(_v_samplesInWindow) = 0;
         __NEXT(7, Warm_Up);

    case MODE_RECOGNIZE:
         __GET(_v_mode) = MODE_RECOGNIZE;
         __GET(_v_movingCount) = 0;
         __GET(_v_stationaryCount) = 0;
         __GET(_v_totalCount) = 0;
         __GET(_v_samplesInWindow) = 0;
         __NEXT(2, AR_Sample);

    default:
        __NEXT(1, Select_Mode);
    }


    // =================================================================
    __BUILDIN_TASK_BOUNDARY(2, AR_Sample);
    AR_SingleSample(&aR_sample, &(__GET(ar_v_seed)));

    __GET(_v_window[__GET(_v_samplesInWindow)].x) = (aR_sample).x;
    __GET(_v_window[__GET(_v_samplesInWindow)].y) = (aR_sample).y;
    __GET(_v_window[__GET(_v_samplesInWindow)].z) = (aR_sample).z;
    ++__GET(_v_samplesInWindow);

    if (__GET(_v_samplesInWindow) < AR_ACCEL_WINDOW_SIZE) {
        __NEXT(2, AR_Sample);
    }
    else {
     __GET(_v_samplesInWindow) = 0;
     __NEXT(3, AR_Transform);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(3, AR_Transform);
    for (i = 0; i < AR_ACCEL_WINDOW_SIZE; ++i)
    {
         if (__GET(_v_window[i].x) < AR_SAMPLE_NOISE_FLOOR ||
                 __GET(_v_window[i].y) < AR_SAMPLE_NOISE_FLOOR ||
                       __GET(_v_window[i].z) < AR_SAMPLE_NOISE_FLOOR)
         {
             __GET(_v_window[i].x) =
                     __GET(_v_window[i].x) > AR_SAMPLE_NOISE_FLOOR ? __GET(_v_window[i].x) : 0;
             __GET(_v_window[i].y) =
                     __GET(_v_window[i].y) > AR_SAMPLE_NOISE_FLOOR ? __GET(_v_window[i].y) : 0;
             __GET(_v_window[i].z) =
                     __GET(_v_window[i].z) > AR_SAMPLE_NOISE_FLOOR ? __GET(_v_window[i].z) : 0;
         }
    }
    __NEXT(4, AR_Featurize);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(4, AR_Featurize);
     mean.x = mean.y = mean.z = 0;
     stddev.x = stddev.y = stddev.z = 0;

     for (i = 0; i < AR_ACCEL_WINDOW_SIZE; ++i)
     {
         mean.x += __GET(_v_window[i].x);
         mean.y += __GET(_v_window[i].y);
         mean.z += __GET(_v_window[i].z);
     }

     mean.x >>= 2;
     mean.y >>= 2;
     mean.z >>= 2;

     for (i = 0; i < AR_ACCEL_WINDOW_SIZE; ++i)
     {
         (aR_sample).x = __GET(_v_window[i].x);
         (aR_sample).y = __GET(_v_window[i].y);
         (aR_sample).z = __GET(_v_window[i].z);

         stddev.x += ((aR_sample).x > mean.x)? ((aR_sample).x - mean.x): (mean.x - (aR_sample).x);
         stddev.y += ((aR_sample).y > mean.y)? ((aR_sample).y - mean.y): (mean.y - (aR_sample).y);
         stddev.z += ((aR_sample).z > mean.z)? ((aR_sample).z - mean.z): (mean.z - (aR_sample).z);

     }

     stddev.x >>= 2;
     stddev.y >>= 2;
     stddev.z >>= 2;

     __GET(_v_meanmag) = mean.x * mean.x + mean.y * mean.y + mean.z * mean.z;
     __GET(_v_stddevmag) = stddev.x * stddev.x + stddev.y * stddev.y + stddev.z * stddev.z;

     features.meanmag = sqrt16(__GET(_v_meanmag));
     features.stddevmag = sqrt16(__GET(_v_stddevmag));

     switch (__GET(_v_mode))
     {
     case MODE_TRAIN_STATIONARY:
     case MODE_TRAIN_MOVING:
         __GET(_v_features.meanmag) = features.meanmag;
         __GET(_v_features.stddevmag) = features.stddevmag;
         __NEXT(8, AR_Train);
     case MODE_RECOGNIZE:
         __GET(_v_features.meanmag) = features.meanmag;
         __GET(_v_features.stddevmag) = features.stddevmag;
         __NEXT(5, AR_Classify);
     }


     // =================================================================
     __BUILDIN_TASK_BOUNDARY(5, AR_Classify);
     move_less_error = 0;
     stat_less_error = 0;
     l_meanmag = __GET(_v_features.meanmag);
     l_stddevmag = __GET(_v_features.stddevmag);


     for (i = 0; i < AR_MODEL_SIZE; ++i)
     {
         ms.meanmag = __GET(_v_model_stationary[i].meanmag);
         ms.stddevmag = __GET(_v_model_stationary[i].stddevmag);
         mm.meanmag = __GET(_v_model_moving[i].meanmag);
         mm.stddevmag = __GET(_v_model_moving[i].stddevmag);

         stat_mean_err = (ms.meanmag > l_meanmag)
             ? (ms.meanmag - l_meanmag)
             : (l_meanmag - ms.meanmag);

         stat_sd_err = (ms.stddevmag > l_stddevmag)
             ? (ms.stddevmag - l_stddevmag)
             : (l_stddevmag - ms.stddevmag);

         move_mean_err = (mm.meanmag > l_meanmag)
             ? (mm.meanmag - l_meanmag)
             : (l_meanmag - mm.meanmag);

         move_sd_err = (mm.stddevmag > l_stddevmag)
             ? (mm.stddevmag - l_stddevmag)
             : (l_stddevmag - mm.stddevmag);

         if (move_mean_err < stat_mean_err)
             move_less_error++;
         else
             stat_less_error++;

         if (move_sd_err < stat_sd_err)
             move_less_error++;
         else
             stat_less_error++;
     }

     __GET(_v_class) = (move_less_error > stat_less_error) ? CLASS_MOVING : CLASS_STATIONARY;
     __NEXT(6, AR_Stats);

     // =================================================================
     __BUILDIN_TASK_BOUNDARY(6, AR_Stats);
     __GET(_v_totalCount)++;
     switch(__GET(_v_class))
     {
     case CLASS_MOVING:
         __GET(_v_movingCount)++;
         break;
     case CLASS_STATIONARY:
         __GET(_v_stationaryCount)++;
         break;
     }

     if (__GET(_v_totalCount) == AR_SAMPLES_TO_COLLECT)
     {
         __GET(resultStationaryPct[__GET(i_debug)]) =
                 __GET(_v_stationaryCount) * 100 / __GET(_v_totalCount);

         __GET(resultMovingPct[__GET(i_debug)]) =
                 __GET(_v_movingCount) * 100 / __GET(_v_totalCount);

         __GET(sum[__GET(i_debug)]) =
                 __GET(_v_stationaryCount) + __GET(_v_movingCount);

         __GET(i_debug)++;
         __NEXT(1, Select_Mode);
     }
     else {
         __NEXT(2, AR_Sample);
     }

     // =================================================================
     __BUILDIN_TASK_BOUNDARY(7, Warm_Up);
     if (__GET(_v_discardedSamplesCount) < AR_NUM_WARMUP_SAMPLES)
     {
         AR_SingleSample(&tA8_sample, &(__GET(ar_v_seed)));
         __GET(_v_discardedSamplesCount)++;
         __NEXT(7, Warm_Up);
     }
     else
     {
         __GET(_v_trainingSetSize) = 0;
         __NEXT(2, AR_Sample);
     }

     // =================================================================
     __BUILDIN_TASK_BOUNDARY(8, AR_Train);
     switch(__GET(_v_class))
     {
     case CLASS_STATIONARY:
         __GET(_v_model_stationary[__GET(_v_trainingSetSize)].meanmag) =
                 __GET(_v_features.meanmag);
         __GET(_v_model_stationary[__GET(_v_trainingSetSize)].stddevmag) =
                 __GET(_v_features.stddevmag);
         break;
     case CLASS_MOVING:
         __GET(_v_model_moving[__GET(_v_trainingSetSize)].meanmag) =
                 __GET(_v_features.meanmag);
         __GET(_v_model_moving[__GET(_v_trainingSetSize)].stddevmag) =
                 __GET(_v_features.stddevmag);
         break;
     }

     __GET(_v_trainingSetSize)++;
     if(__GET(_v_trainingSetSize) < AR_MODEL_SIZE) {
         __NEXT(2, AR_Sample);
     }
     else {
         __NEXT(1, Select_Mode);
     }

}
