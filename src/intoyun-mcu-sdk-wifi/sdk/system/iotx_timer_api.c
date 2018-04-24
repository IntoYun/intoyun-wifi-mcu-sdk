#include "intoyun_timer.h"
#include "intoyun_log.h"
#include "intoyun_config.h"
#include "intoyun_interface.h"

#ifdef CONFIG_INTOYUN_TIMER

static timer_t *timerListHead = NULL;
static bool timerListInitFlag = false;

static void TimerListInit(void)
{
    timerListHead = (timer_t*)malloc(sizeof(timer_t));
    if(timerListHead == NULL){
        return;
    }
    timerListHead->next = NULL;
}

//该按键是否存在
static bool TimerExists( timer_t *obj )
{
    timer_t* cur = timerListHead->next;

    while( cur != NULL )
    {
        if( cur->timerNum == obj->timerNum )
        {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

//按键插入
static void TimerListInsert(timer_t *obj)
{
    timer_t *head = timerListHead;
    timer_t *cur = timerListHead->next;

    if( ( obj == NULL ) || ( TimerExists( obj ) == true ) ){
        log_v("timer is exists\r\n");
        return;
    }

    if(cur == NULL) {
        head->next = obj;
        obj->next = NULL;
    }else{
        while((cur->next != NULL)){
            cur = cur->next;
        }
        cur->next = obj;
        obj->next = NULL;
    }
}

static void TimerListChangPeriod(uint8_t num, uint32_t period)
{
    timer_t* cur = timerListHead->next;

    while( cur != NULL )
    {
        if( cur->timerNum == num )
        {
            cur->period = period;
        }
        cur = cur->next;
    }
}

static void TimerListTimerStart(uint8_t num)
{
    timer_t* cur = timerListHead->next;

    while( cur != NULL )
    {
        if( cur->timerNum == num )
        {
            cur->start = true;
        }
        cur = cur->next;
    }
}

static void TimerListTimerStop(uint8_t num)
{
    timer_t* cur = timerListHead->next;

    while( cur != NULL )
    {
        if( cur->timerNum == num )
        {
            cur->start = false;
        }
        cur = cur->next;
    }
}

static void TimerListTimerReset(uint8_t num)
{
    timer_t* cur = timerListHead->next;

    while( cur != NULL )
    {
        if( cur->timerNum == num )
        {
            cur->timerTick = 0;
        }
        cur = cur->next;
    }
}

static void TimerListLoop(void)
{
    timer_t* head = timerListHead;
    timer_t* cur = timerListHead->next;

    while( cur != NULL && head != NULL)
    {
        if(cur->start)
        {
            if(++cur->timerTick >= cur->period)
            {
                cur->timerTick = 0;
                if(cur->timerCbFunc != NULL)
                {
                    cur->timerCbFunc();
                }
                if(cur->oneShot) //只执行一次
                {
                    cur->start = false;
                }
            }
        }
        cur = cur->next;
    }
}

void intoyunTimerRegister(uint8_t num, uint32_t period, bool oneShot, cbTimerFunc cbFunc)
{
    if(!timerListInitFlag){
        timerListInitFlag = true;
        TimerListInit();
    }

    timer_t *p = (timer_t*)malloc(sizeof(timer_t));
    if(p == NULL){
        log_v("error malloc\r\n");
        return;
    }
    p->timerNum= num;
    p->period= period;
    p->oneShot = oneShot;
    p->start = false;
    p->timerTick = 0;
    p->timerCbFunc = cbFunc;
    p->next = NULL;

    log_v("timerNum=%d\r\n",p->timerNum);

    TimerListInsert(p);
}

void intoyunTimerChangePeriod(uint8_t num, uint32_t period)
{
    TimerListChangPeriod(num, period);
}

void intoyunTimerStart(uint8_t num)
{
    TimerListTimerStart(num);
}

void intoyunTimerStop(uint8_t num)
{
    TimerListTimerStop(num);
}

void intoyunTimerReset(uint8_t num)
{
    TimerListTimerReset(num);
}

void intoyunTimerLoop(void)
{
    TimerListLoop();
}

#endif
