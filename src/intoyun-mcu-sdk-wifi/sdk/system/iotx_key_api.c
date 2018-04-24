#include "intoyun_key.h"
#include "intoyun_log.h"
#include "intoyun_config.h"
#include "intoyun_interface.h"

#ifdef CONFIG_INTOYUN_KEY

static key_param_t keyParams = {0,20,600,1000,1,0,false,0,0};
static uint8_t keyNumRecord = 0;  //记录按下了的按键编号
static key_t *keyListHead = NULL;
static bool keyListInitFlag = false;

static void KeyListInit(void)
{
    keyListHead = (key_t*)malloc(sizeof(key_t));
    if(keyListHead == NULL){
        return;
    }
    keyListHead->next = NULL;
}

//该按键是否存在
static bool KeyExists( key_t *obj )
{
    key_t* cur = keyListHead->next;

    while( cur != NULL )
    {
        if( cur->keyNum == obj->keyNum )
        {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

//按键插入
static void KeyListInsert(key_t *obj)
{
    key_t *head = keyListHead;
    key_t *cur = keyListHead->next;

    if( ( obj == NULL ) || ( KeyExists( obj ) == true ) ){
        log_v("key is exists\r\n");
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

static bool KeyListQueryDoubleClickCb(uint8_t num)
{
    key_t *cur = keyListHead->next;
    while(cur != NULL){
        if( cur->keyNum == num)
        {
            if(cur->cbKeyDoubleClickFunc != NULL) {
                return true;
            }
        }
        cur = cur->next;
    }
    return false;
}

//查找按键 注册按键回调
static void KeyListRegisterCbFunc(uint8_t cbType, uint8_t num, cbPressFunc cbFunc, cbClickFunc clickFunc)
{
    key_t *cur = keyListHead->next;
    while(cur != NULL){
        if( cur->keyNum == num)
        {
            switch(cbType){
            case KEY_CLICK_CB:
                cur->cbKeyClickFunc = clickFunc;
                return;
            case KEY_DOUBLE_CLICK_CB:
                cur->cbKeyDoubleClickFunc = clickFunc;
                return;
            case KEY_PRESS_SATRT_CB:
                cur->cbKeyPressStartFunc = cbFunc;
                return;
            case KEY_PRESS_STOP_CB:
                cur->cbKeyPressStopFunc = cbFunc;
                return;
            case KEY_PRESS_DURING_CB:
                cur->cbKeyPressDuringFunc = cbFunc;
                return;
            default:
                return;
            }
        }
        cur = cur->next;
    }
}

//执行按键回调函数
static void KeyListExeCbFunc(uint8_t cbType, uint8_t num, uint32_t ms)
{
    key_t *cur = keyListHead->next;
    while(cur != NULL){
        if( cur->keyNum == num)
        {
            switch(cbType){
            case KEY_CLICK_CB:
                if(cur->cbKeyClickFunc != NULL) {
                    cur->cbKeyClickFunc();
                }
                break;
            case KEY_DOUBLE_CLICK_CB:
                if(cur->cbKeyDoubleClickFunc != NULL) {
                    cur->cbKeyDoubleClickFunc();
                }
            case KEY_PRESS_SATRT_CB:
                if(cur->cbKeyPressStartFunc != NULL) {
                    cur->cbKeyPressStartFunc(ms);
                }
                break;
            case KEY_PRESS_STOP_CB:
                if(cur->cbKeyPressStopFunc != NULL) {
                    cur->cbKeyPressStopFunc(ms);
                }
                break;
            case KEY_PRESS_DURING_CB:
                if(cur->cbKeyPressDuringFunc != NULL) {
                    cur->cbKeyPressDuringFunc(ms);
                }
                break;
            default:
                break;
            }
        }
        cur = cur->next;
    }
}

//获取键值
static int KeyListGetValue(void)
{
    key_t *cur = keyListHead->next;
    while(cur != NULL){
        if(cur->cbKeyGetValueFunc() == keyParams._buttonPressed){
            keyNumRecord = cur->keyNum; //记录哪个按键按下了
            return keyParams._buttonPressed;
        }
        cur = cur->next;
    }
    return keyParams._buttonReleased;
}

void intoyunKeyInit(void)
{
    key_t* cur = keyListHead->next;

    while( cur != NULL )
    {
        if(cur->cbKeyInitFunc != NULL){
            cur->cbKeyInitFunc();
        }
        cur = cur->next;
    }
}

void intoyunKeySetParams(bool invert, uint32_t debounceTime, uint32_t clickTime, uint32_t pressTime)
{
    if(!invert) {
        keyParams._buttonPressed = 0;
        keyParams._buttonReleased = 1;
    } else {
        keyParams._buttonPressed = 1;
        keyParams._buttonReleased = 0;
    }
    keyParams._debounceTime = debounceTime;
    keyParams._clickTime = clickTime;
    keyParams._pressTime = pressTime;
    keyParams._state = 0;
    keyParams._startTime = 0;
    keyParams._stopTime = 0;
}

void intoyunKeyRegister(uint8_t num, cbInitFunc initFunc, cbGetValueFunc getValFunc)
{
    if(!keyListInitFlag){
        keyListInitFlag = true;
        KeyListInit();
    }

    key_t *p = (key_t*)malloc(sizeof(key_t));
    if(p == NULL){
        log_v("error malloc\r\n");
        return;
    }
    p->keyNum= num;
    p->cbKeyInitFunc = initFunc;
    p->cbKeyGetValueFunc = getValFunc;
    p->cbKeyClickFunc = NULL;
    p->cbKeyDoubleClickFunc = NULL;
    p->cbKeyPressStartFunc = NULL;
    p->cbKeyPressStopFunc = NULL;
    p->cbKeyPressDuringFunc = NULL;
    p->next = NULL;

    log_v("keyNum=%d\r\n",p->keyNum);

    KeyListInsert(p);
}

//单击
void intoyunKeyClickCb(uint8_t num, cbClickFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_CLICK_CB, num, NULL, cbFunc);
}

//双击
void intoyunKeyDoubleClickCb(uint8_t num, cbClickFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_DOUBLE_CLICK_CB, num, NULL, cbFunc);
}

//长按键开始按下
void intoyunKeyPressStartCb(uint8_t num, cbPressFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_PRESS_SATRT_CB, num, cbFunc, NULL);
}

//长按键松开
void intoyunKeyPressStopCb(uint8_t num, cbPressFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_PRESS_STOP_CB, num, cbFunc, NULL);
}

//按键一直长按
void intoyunKeyPressDuringCb(uint8_t num, cbPressFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_PRESS_DURING_CB, num, cbFunc, NULL);
}

void intoyunKeyLoop(void)
{
    // Detect the input information
    int buttonLevel = KeyListGetValue(); // current button signal.
    uint32_t now = millis(); // current (relative) time in msecs.

    // Implementation of the state machine
    if (keyParams._state == 0) {  // waiting for menu pin being pressed.
        if (buttonLevel == keyParams._buttonPressed) {
            keyParams._state = 1; // step to state 1
            keyParams._startTime = now; // remember starting time
        }
    } else if (keyParams._state == 1) { // waiting for menu pin being released.
        if ((buttonLevel == keyParams._buttonReleased) && ((uint32_t)(now - keyParams._startTime) < keyParams._debounceTime)) {
            // button was released to quickly so I assume some debouncing.
            // go back to state 0 without calling a function.
            keyParams._state = 0;
        } else if (buttonLevel == keyParams._buttonReleased) {
            keyParams._state = 2; // step to state 2
            keyParams._stopTime = now; // remember stopping time
        } else if ((buttonLevel == keyParams._buttonPressed) && ((uint32_t)(now - keyParams._startTime) > keyParams._pressTime)) {
            keyParams._isLongPressed = true;  // Keep track of long press state
            KeyListExeCbFunc(KEY_PRESS_SATRT_CB, keyNumRecord, 0);
            KeyListExeCbFunc(KEY_PRESS_DURING_CB, keyNumRecord, now - keyParams._startTime);
            keyParams._state = 6; // step to state 6
        } else {
            // wait. Stay in this state.
        }
    } else if (keyParams._state == 2) { // waiting for menu pin being pressed the second time or timeout.
        if (KeyListQueryDoubleClickCb(keyNumRecord) == false || (uint32_t)(now - keyParams._startTime) > keyParams._clickTime) {
            // this was only a single short click
            KeyListExeCbFunc(KEY_CLICK_CB, keyNumRecord, 0);
            keyParams._state = 0; // restart.
        } else if ((buttonLevel == keyParams._buttonPressed) && ((uint32_t)(now - keyParams._stopTime) > keyParams._debounceTime)) {
            keyParams._state = 3; // step to state 3
            keyParams._startTime = now; // remember starting time
        }
    } else if (keyParams._state == 3) { // waiting for menu pin being released finally.
        // Stay here for at least keyParams._debounceTime because else we might end up in state 1 if the
        // button bounces for too long.
        if (buttonLevel == keyParams._buttonReleased && ((uint32_t)(now - keyParams._startTime) > keyParams._debounceTime)) {
            // this was a 2 click sequence.
            KeyListExeCbFunc(KEY_DOUBLE_CLICK_CB, keyNumRecord, 0);
            keyParams._state = 0; // restart.
        }
    } else if (keyParams._state == 6) { // waiting for menu pin being release after long press.
        if (buttonLevel == keyParams._buttonReleased) {
            keyParams._isLongPressed = false;  // Keep track of long press state
            KeyListExeCbFunc(KEY_PRESS_STOP_CB, keyNumRecord, now - keyParams._startTime);
            keyParams._state = 0; // restart.
        } else {
            // button is being long pressed
            keyParams._isLongPressed = true; // Keep track of long press state
            KeyListExeCbFunc(KEY_PRESS_DURING_CB, keyNumRecord, now - keyParams._startTime);
        }
    }
}

#endif
