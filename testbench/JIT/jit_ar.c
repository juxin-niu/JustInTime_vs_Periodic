#include <testbench/global_declaration.h>
#include <testbench/testbench_api.h>

// Shared Varaibles

__nv uint16_t        i_debug;
__nv uint16_t        ar_v_seed;

__nv uint16_t        _v_count;
__nv uint16_t        _v_pinState;
__nv uint16_t        _v_samplesInWindow;
__nv uint16_t        _v_totalCount;
__nv uint16_t        _v_movingCount;
__nv uint16_t        _v_stationaryCount;
__nv uint16_t        _v_discardedSamplesCount;
__nv uint16_t        _v_trainingSetSize;

__nv ar_class_t      _v_class;
__nv ar_run_mode_t   _v_mode;
__nv ar_features_t   _v_features;
__nv uint16_t        _v_meanmag;
__nv uint16_t        _v_stddevmag;
__nv accelReading    _v_window[AR_ACCEL_WINDOW_SIZE];
__nv ar_features_t   _v_model_stationary[AR_MODEL_SIZE];
__nv ar_features_t   _v_model_moving[AR_MODEL_SIZE];

__nv uint16_t resultStationaryPct[4];
__nv uint16_t resultMovingPct[4];
__nv uint16_t sum[4];

void jit_ar_main()
{
    uint16_t i;
    uint16_t pin_state;         // Used in Select_Mode
    threeAxis_t_8 tA8_sample;   // Used in Warm_Up
    accelReading aR_sample;     // Used in AR_Sample and AR_Featurize
    accelReading mean, stddev;  // Used in AR_Featurize
    ar_features_t features;     // Used in AR_Featurize
    int16_t move_less_error;    // Used in AR_Classify
    int16_t stat_less_error;    // Used in AR_Classify
    int32_t l_meanmag;          // Used in AR_Classify
    int32_t l_stddevmag;        // Used in AR_Classify
    ar_features_t ms, mm;       // Used in AR_Classify

    i_debug = 0;
    ar_v_seed = 0x1111;
    _v_pinState = MODE_IDLE;
    _v_count = 0;

    // ******************************************************************************
    Select_Mode:
    pin_state = MODE_TRAIN_MOVING;  // 1
    _v_count++;

    if(_v_count >= 7)            goto quit;
    else if(_v_count >= 3)       pin_state = MODE_RECOGNIZE;        // 0
    else if(_v_count >= 2)       pin_state = MODE_TRAIN_STATIONARY; // 2

    if ( (pin_state == MODE_TRAIN_STATIONARY || pin_state == MODE_TRAIN_MOVING)
            && (pin_state == _v_pinState) ) {
        pin_state = MODE_IDLE;
    }
    else {
        _v_pinState = pin_state;
    }

    switch(pin_state)
    {
    case MODE_TRAIN_STATIONARY:
        _v_discardedSamplesCount = 0;
        _v_mode = MODE_TRAIN_STATIONARY;
        _v_class = CLASS_STATIONARY;
        _v_samplesInWindow = 0;
        goto Warm_Up;

    case MODE_TRAIN_MOVING:
        _v_discardedSamplesCount = 0;
        _v_mode = MODE_TRAIN_MOVING;
        _v_class = CLASS_MOVING;
        _v_samplesInWindow = 0;
        goto Warm_Up;

    case MODE_RECOGNIZE:
        _v_mode = MODE_RECOGNIZE;
        _v_movingCount = 0;
        _v_stationaryCount = 0;
        _v_totalCount = 0;
        _v_samplesInWindow = 0;
        goto AR_Sample;

    default:
        goto Select_Mode;
    }


    // ******************************************************************************
    Warm_Up:
    if (_v_discardedSamplesCount < AR_NUM_WARMUP_SAMPLES)
    {
        AR_SingleSample(&tA8_sample, &ar_v_seed);
        _v_discardedSamplesCount++;
        goto Warm_Up;
    }
    else
    {
        _v_trainingSetSize = 0;
        goto AR_Sample;
    }


    // ******************************************************************************
    AR_Sample:
    AR_SingleSample(&aR_sample, &ar_v_seed);
    _v_window[_v_samplesInWindow].x = aR_sample.x;
    _v_window[_v_samplesInWindow].y = aR_sample.y;
    _v_window[_v_samplesInWindow].z = aR_sample.z;
    ++_v_samplesInWindow;

    if (_v_samplesInWindow < AR_ACCEL_WINDOW_SIZE)
        goto AR_Sample;
    else
    {
        _v_samplesInWindow = 0;
        goto AR_Transform;
    }

    // ******************************************************************************
    AR_Transform:
    for (i = 0; i < AR_ACCEL_WINDOW_SIZE; ++i)
    {
        if (_v_window[i].x < AR_SAMPLE_NOISE_FLOOR ||
                _v_window[i].y < AR_SAMPLE_NOISE_FLOOR ||
                _v_window[i].z < AR_SAMPLE_NOISE_FLOOR)
        {
            _v_window[i].x = _v_window[i].x > AR_SAMPLE_NOISE_FLOOR ? _v_window[i].x : 0;
            _v_window[i].y = _v_window[i].y > AR_SAMPLE_NOISE_FLOOR ? _v_window[i].y : 0;
            _v_window[i].z = _v_window[i].z > AR_SAMPLE_NOISE_FLOOR ? _v_window[i].z : 0;
        }
    }
    goto AR_Featurize;

    // ******************************************************************************
    AR_Featurize:
    mean.x = mean.y = mean.z = 0;
    stddev.x = stddev.y = stddev.z = 0;

    for (i = 0; i < AR_ACCEL_WINDOW_SIZE; ++i)
    {
        mean.x += _v_window[i].x;
        mean.y += _v_window[i].y;
        mean.z += _v_window[i].z;
    }

    mean.x >>= 2;
    mean.y >>= 2;
    mean.z >>= 2;

    for (i = 0; i < AR_ACCEL_WINDOW_SIZE; ++i)
    {
        aR_sample.x = _v_window[i].x;
        aR_sample.y = _v_window[i].y;
        aR_sample.z = _v_window[i].z;

        stddev.x += (aR_sample.x > mean.x)? (aR_sample.x - mean.x): (mean.x - aR_sample.x);
        stddev.y += (aR_sample.y > mean.y)? (aR_sample.y - mean.y): (mean.y - aR_sample.y);
        stddev.z += (aR_sample.z > mean.z)? (aR_sample.z - mean.z): (mean.z - aR_sample.z);

    }

    stddev.x >>= 2;
    stddev.y >>= 2;
    stddev.z >>= 2;

    _v_meanmag = mean.x * mean.x + mean.y * mean.y + mean.z * mean.z;
    _v_stddevmag = stddev.x * stddev.x + stddev.y * stddev.y + stddev.z * stddev.z;

    features.meanmag = sqrt16(_v_meanmag);
    features.stddevmag = sqrt16(_v_stddevmag);

    switch (_v_mode)
    {
    case MODE_TRAIN_STATIONARY:
    case MODE_TRAIN_MOVING:
        _v_features.meanmag = features.meanmag;
        _v_features.stddevmag = features.stddevmag;
        goto AR_Train;
    case MODE_RECOGNIZE:
        _v_features.meanmag = features.meanmag;
        _v_features.stddevmag = features.stddevmag;
        goto AR_Classify;
    }

    AR_Classify:
    move_less_error = 0;
    stat_less_error = 0;
    l_meanmag = _v_features.meanmag;
    l_stddevmag = _v_features.stddevmag;

    for (i = 0; i < AR_MODEL_SIZE; ++i)
    {
        ms.meanmag = _v_model_stationary[i].meanmag;
        ms.stddevmag =_v_model_stationary[i].stddevmag;
        mm.meanmag = _v_model_moving[i].meanmag;
        mm.stddevmag = _v_model_moving[i].stddevmag;

        int32_t stat_mean_err = (ms.meanmag > l_meanmag)
            ? (ms.meanmag - l_meanmag)
            : (l_meanmag - ms.meanmag);

        int32_t stat_sd_err = (ms.stddevmag > l_stddevmag)
            ? (ms.stddevmag - l_stddevmag)
            : (l_stddevmag - ms.stddevmag);

        int32_t move_mean_err = (mm.meanmag > l_meanmag)
            ? (mm.meanmag - l_meanmag)
            : (l_meanmag - mm.meanmag);

        int32_t move_sd_err = (mm.stddevmag > l_stddevmag)
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

    _v_class = (move_less_error > stat_less_error) ? CLASS_MOVING : CLASS_STATIONARY;

    _v_totalCount++;
    switch(_v_class)
    {
    case CLASS_MOVING:
        _v_movingCount++;
        break;
    case CLASS_STATIONARY:
        _v_stationaryCount++;
        break;
    }

    if (_v_totalCount == AR_SAMPLES_TO_COLLECT)
    {
        resultStationaryPct[i_debug] = _v_stationaryCount * 100 / _v_totalCount;
        resultMovingPct[i_debug] = _v_movingCount * 100 / _v_totalCount;
        sum[i_debug] = _v_stationaryCount + _v_movingCount;
        i_debug++;
        goto Select_Mode;
    }
    else
        goto AR_Sample;

    AR_Train:
    switch(_v_class)
    {
    case CLASS_STATIONARY:
        _v_model_stationary[_v_trainingSetSize].meanmag = _v_features.meanmag;
        _v_model_stationary[_v_trainingSetSize].stddevmag = _v_features.stddevmag;
        break;
    case CLASS_MOVING:
        _v_model_moving[_v_trainingSetSize].meanmag = _v_features.meanmag;
        _v_model_moving[_v_trainingSetSize].stddevmag = _v_features.stddevmag;
        break;
    }

    _v_trainingSetSize++;
    if(_v_trainingSetSize < AR_MODEL_SIZE)
        goto AR_Sample;
    else
        goto Select_Mode;

    quit:
    return;
}
