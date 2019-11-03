/*
 * Copyright (c) 2019 Martin Rödel a.k.a. Yomin Nimoy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <ytil/test/state.h>
#include <ytil/def.h>
#include <ytil/ext/stdio.h>
#include <ytil/ext/string.h>
#include <ytil/con/vec.h>


typedef struct test_state
{
    test_status_id status;
    test_result_id result;
    size_t duration, asserts;
    test_pos_st pos;
    vec_ct msg;
} test_state_st;

typedef struct test_fold_state
{
    test_state_msg_cb fold;
    void *ctx;
} test_fold_state_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_TEST_STATE_INVALID_OBJECT, "Invalid test state object.")
    , ERROR_INFO(E_TEST_STATE_INVALID_STATUS_TYPE, "Invalid status type.")
    , ERROR_INFO(E_TEST_STATE_INVALID_RESULT_TYPE, "Invalid result type.")
    , ERROR_INFO(E_TEST_STATE_INVALID_FILE, "Invalid file.")
    , ERROR_INFO(E_TEST_STATE_INVALID_MSG, "Invalid message.")
    , ERROR_INFO(E_TEST_STATE_INVALID_MSG_TYPE, "Invalid message type.")
    , ERROR_INFO(E_TEST_STATE_INVALID_CALLBACK, "Invalid callback.")
};


test_state_ct test_state_new(void)
{
    test_state_ct state;
    
    if(!(state = calloc(1, sizeof(test_state_st))))
        return error_wrap_errno(calloc), NULL;
    
    return state;
}

void test_state_free(test_state_ct state)
{
    test_state_reset(state);
    
    if(state->msg)
        vec_free(state->msg);
    
    free(state);
}

static void test_state_vec_free_msg(vec_const_ct vec, void *elem, void *ctx)
{
    test_msg_st *msg = elem;
    
    free(msg->text);
    
    if(msg->pos.file)
        free(msg->pos.file);
}

void test_state_reset(test_state_ct state)
{
    assert(state);
    
    if(state->msg)
        vec_clear_f(state->msg, test_state_vec_free_msg, NULL);
    
    if(state->pos.file)
        free(state->pos.file);
    
    state->status = TEST_STATUS_INIT;
    state->result = TEST_RESULT_PASS;
    state->duration = 0;
    state->asserts = 0;
    state->pos.file = NULL;
}

int test_state_set_status(test_state_ct state, test_status_id status)
{
    return_error_if_fail(state, E_TEST_STATE_INVALID_OBJECT, -1);
    return_error_if_fail(status < TEST_STATUSES, E_TEST_STATE_INVALID_STATUS_TYPE, -1);
    
    state->status = status;
    
    return 0;
}

int test_state_set_result(test_state_ct state, test_result_id result)
{
    return_error_if_fail(state, E_TEST_STATE_INVALID_OBJECT, -1);
    return_error_if_fail(result < TEST_RESULTS, E_TEST_STATE_INVALID_RESULT_TYPE, -1);
    
    state->result = MAX(state->result, result);
    
    return 0;
}

int test_state_add_duration(test_state_ct state, size_t duration)
{
    return_error_if_fail(state, E_TEST_STATE_INVALID_OBJECT, -1);
    
    state->duration += duration;
    
    return 0;
}

int test_state_set_position(test_state_ct state, test_pos_id type, const char *file, size_t line)
{
    return_error_if_fail(state, E_TEST_STATE_INVALID_OBJECT, -1);
    return_error_if_fail(file, E_TEST_STATE_INVALID_FILE, -1);
    
    if(state->pos.file)
        free(state->pos.file);
    
    if(!(state->pos.file = strdup(file)))
        return error_wrap_errno(strdup), -1;
    
    state->pos.type = type;
    state->pos.line = line;
    
    return 0;
}

int test_state_inc_asserts(test_state_ct state)
{
    return_error_if_fail(state, E_TEST_STATE_INVALID_OBJECT, -1);
    
    state->asserts++;
    
    return 0;
}

test_status_id test_state_get_status(test_state_ct state)
{
    assert(state);
    
    return state->status;
}

const char *test_state_get_strstatus(test_state_ct state)
{
    assert(state);
    
    return error_propagate_ptr(test_state_strstatus(state->status));
}

test_result_id test_state_get_result(test_state_ct state)
{
    assert(state);
    
    return state->result;
}

const char *test_state_get_strresult(test_state_ct state)
{
    assert(state);
    
    return error_propagate_ptr(test_state_strresult(state->result));
}

size_t test_state_get_duration(test_state_ct state)
{
    assert(state);
    
    return state->duration;
}

test_pos_st *test_state_get_position(test_state_ct state)
{
    assert(state);
    
    return &state->pos;
}

size_t test_state_get_asserts(test_state_ct state)
{
    assert(state);
    
    return state->asserts;
}

const char *test_state_strstatus(test_status_id status)
{
    switch(status)
    {
    case TEST_STATUS_INIT:      return "init";
    case TEST_STATUS_SETUP:     return "setup";
    case TEST_STATUS_RUN:       return "run";
    case TEST_STATUS_TEARDOWN:  return "teardown";
    case TEST_STATUS_FINISH:    return "finish";
    default:                    return_error_if_reached(E_TEST_STATE_INVALID_STATUS_TYPE, NULL);
    }
}

const char *test_state_strresult(test_result_id result)
{
    switch(result)
    {
    case TEST_RESULT_PASS:      return "pass";
    case TEST_RESULT_WARNING:   return "warning";
    case TEST_RESULT_FAIL:      return "fail";
    case TEST_RESULT_TIMEOUT:   return "timeout";
    case TEST_RESULT_ERROR:     return "error";
    case TEST_RESULT_SKIP:      return "skip";
    default:                    return_error_if_reached(E_TEST_STATE_INVALID_RESULT_TYPE, NULL);
    }
}

static int _test_state_add_msg(test_state_ct state, test_msg_id type, size_t level, const char *msgtext, size_t len)
{
    test_msg_st *msg;
    char *text;
    
    if((!len && !(text = strdup(msgtext))) || (len && !(text = strndup(msgtext, len))))
        return error_wrap_errno(strdup), -1;
    
    if(!state->msg && !(state->msg = vec_new(10, sizeof(test_msg_st))))
        return error_wrap(), free(text), -1;
    
    if(!(msg = vec_push(state->msg)))
        return error_wrap(), free(text), -1;
    
    msg->type = type;
    msg->level = level;
    msg->text = text;
    
    if(!level)
    {
        msg->pos.type = state->pos.type;
        msg->pos.line = state->pos.line;
        msg->pos.file =
            state->pos.file ? strdup(state->pos.file) : NULL; // let it fail
    }
    
    return 0;
}

int test_state_add_msg(test_state_ct state, test_msg_id type, const char *msg)
{
    const char *ptr;
    size_t level = 0;
    int rc = 0;
    
    return_error_if_fail(state, E_TEST_STATE_INVALID_OBJECT, -1);
    return_error_if_fail(msg, E_TEST_STATE_INVALID_MSG, -1);
    return_error_if_fail(type < TEST_MSG_TYPES, E_TEST_STATE_INVALID_MSG_TYPE, -1);
    
    switch(type)
    {
    case TEST_MSG_INFO:    break;
    case TEST_MSG_WARNING: rc = test_state_set_result(state, TEST_RESULT_WARNING); break;
    case TEST_MSG_ERROR:   rc = test_state_set_result(state, TEST_RESULT_FAIL); break;
    default:               abort();
    }
    
    if(rc)
        return error_propagate(), rc;
    
    for(; (ptr = strchr(msg, '\n')); msg = ptr+1)
        if(ptr == msg)
            continue;
        else if(_test_state_add_msg(state, type, level, msg, ptr-msg))
            return error_propagate(), -1;
        else
            level = 1;
    
    if(!msg[0])
        return 0;
    
    return error_propagate_int(_test_state_add_msg(state, type, level, msg, 0));
}

int test_state_add_msg_f(test_state_ct state, test_msg_id type, const char *fmt, ...)
{
    return error_propagate_int(test_state_add_msg(state, type, VVFMT(fmt)));
}

static int test_state_vec_fold_msg(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    test_fold_state_st *fstate = ctx;
    test_msg_st *msg = elem;
    
    return error_wrap_int(fstate->fold(&msg->pos, msg->type, msg->level, msg->text, fstate->ctx));
}

int test_state_fold_msg(test_state_ct state, test_state_msg_cb fold, void *ctx)
{
    test_fold_state_st fstate = { .fold = fold, .ctx = ctx };
    
    return_error_if_fail(state, E_TEST_STATE_INVALID_OBJECT, -1);
    return_error_if_fail(fold, E_TEST_STATE_INVALID_CALLBACK, -1);
    
    if(!state->msg)
        return 0;
    
    return error_wrap_int(vec_fold(state->msg, test_state_vec_fold_msg, &fstate));
}
