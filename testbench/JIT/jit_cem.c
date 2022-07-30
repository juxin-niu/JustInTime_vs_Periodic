#include <testbench/global_declaration.h>
#include <testbench/testbench_api.h>

// Shared Variables
 __nv uint16_t       _v_letter_idx;
 __nv cem_sample_t   _v_prev_sample;
 __nv cem_index_t    _v_sample_count;
 __nv cem_index_t    _v_sibling;
 __nv cem_index_t    _v_node_count;
 __nv cem_index_t    _v_out_len;

 __nv cem_letter_t   _v_letter;
 __nv cem_sample_t   _v_sample;
 __nv cem_index_t    _v_child;
 __nv cem_index_t    _v_parent;
 __nv cem_index_t    _v_parent_next;
 __nv cem_node_t     _v_parent_node;
 __nv cem_node_t     _v_sibling_node;
 __nv cem_index_t    _v_symbol;

 __nv cem_node_t     _v_compressed_data[CEM_BLOCK_SIZE];
 __nv cem_node_t     _v_dict[CEM_DICT_SIZE];

void jit_cem_main()
{
    uint16_t i;
    uint16_t next_letter_idx;
    cem_sample_t prev_sample;
    cem_sample_t sample;
    uint16_t letter_idx;
    uint16_t letter_shift;
    cem_letter_t letter;
    cem_index_t parent;
    cem_index_t starting_node_idx;
    cem_index_t child;


    _v_parent_next = 0;
    _v_out_len = 0;
    _v_letter = 0;
    _v_prev_sample = 0;
    _v_letter_idx = 0;
    _v_sample_count = 1;


    for (i = 0; i < CEM_NUM_LETTERS; ++i)
    {
        _v_dict[i].letter = i;
        _v_dict[i].sibling = CEM_NIL;
        _v_dict[i].child = CEM_NIL;
    }

    _v_letter = CEM_NUM_LETTERS + 1;
    _v_node_count = CEM_NUM_LETTERS;

    Sample:
    next_letter_idx = _v_letter_idx + 1;
    if (next_letter_idx == CEM_NUM_LETTERS_IN_SAMPLE)
        next_letter_idx = 0;

    if (_v_letter_idx == 0)
    {
        _v_letter_idx = next_letter_idx;
        goto Measure_Temp;
    }
    else
    {
        _v_letter_idx = next_letter_idx;
        goto Letterize_and_compress;
    }

    Measure_Temp:
    prev_sample = _v_prev_sample;
    sample = CEM_AcquireSample(prev_sample);
    prev_sample = sample;
    _v_prev_sample = prev_sample;
    _v_sample = sample;

    Letterize_and_compress:
    letter_idx = _v_letter_idx;
    if (letter_idx == 0)
        letter_idx = CEM_NUM_LETTERS_IN_SAMPLE;
    else
        letter_idx--;

    letter_shift = CEM_LETTER_SIZE_BITS * letter_idx;
    letter = ( _v_sample & (CEM_LETTER_MASK << letter_shift) ) >> letter_shift;
    _v_letter = letter;

    parent = _v_parent_next;
    _v_parent_node.letter = _v_dict[parent].letter;
    _v_parent_node.sibling = _v_dict[parent].sibling;
    _v_parent_node.child = _v_dict[parent].child;
    _v_sibling = _v_dict[parent].child;
    _v_parent = parent;
    _v_child = _v_dict[parent].child;
    _v_sample_count++;

    Find_Sibling:
    if (_v_sibling != CEM_NIL)
    {
        int16_t idx = _v_sibling;
        if(_v_dict[idx].letter == _v_letter)
        {
            _v_parent_next = _v_sibling;
            goto Letterize_and_compress;
        }
        else
        {
            if (_v_dict[idx].sibling != 0)
            {
                _v_sibling = _v_dict[idx].sibling;
                goto Find_Sibling;
            }
        }
    }

    starting_node_idx = (cem_index_t)_v_letter;
    _v_parent_next = starting_node_idx;
    if (_v_child == CEM_NIL)
        goto Add_Insert;
    else
        goto Add_Node;


    Add_Node:
    i = _v_sibling;
    if (_v_dict[i].sibling != CEM_NIL)
    {
        cem_index_t next_sibling = _v_dict[i].sibling;
        _v_sibling = next_sibling;
        goto Add_Node;
    }
    else
    {
        _v_sibling_node.letter = _v_dict[i].letter;
        _v_sibling_node.sibling = _v_dict[i].sibling;
        _v_sibling_node.child = _v_dict[i].child;
        goto Add_Insert;
    }

    Add_Insert:
    if (_v_node_count == CEM_DICT_SIZE)
        while(1){}

    child = _v_node_count;
    if (_v_parent_node.child == CEM_NIL)
    {
        int16_t i = _v_parent;
        _v_dict[i].letter = _v_parent_node.letter;
        _v_dict[i].sibling = _v_parent_node.sibling;
        _v_dict[i].child = child;
    }
    else
    {
        cem_index_t last_sibling = _v_sibling;
        _v_dict[last_sibling].letter = _v_sibling_node.letter;
        _v_dict[last_sibling].sibling = child;
        _v_dict[last_sibling].child = _v_sibling_node.child;
    }

    _v_dict[child].letter = _v_letter;
    _v_dict[child].sibling = CEM_NIL;
    _v_dict[child].child = CEM_NIL;
    _v_symbol = _v_parent;
    _v_node_count++;

    _v_compressed_data[_v_out_len].letter = _v_symbol;
    _v_out_len++;
    if(_v_out_len == CEM_BLOCK_SIZE)
        return;
    else
        goto Sample;
}
