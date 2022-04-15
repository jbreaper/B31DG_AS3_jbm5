/* Embbeded Software - Assignment 3
 * This project was programmed for for the ESP32 microcontroller, it uses freeRTOS to run a series of tasks 
 * based on the provided brief.
 *
 * -------
 *  BRIEF
 * -------
 * For this assignment the tasks created in assignment 2 were recreated to utilise the freeRTOS framework.
 * An additional requirement was added for this assignment, that being that in order for task 9 to print
 * the data, the button from task 2 must be pressed.
 * This the RTOS system must complete the following tasks at the related frequencies.
 * __________________________________________________________________________________________________________________________
 * |  Task  | Description                                                                       | Frequency   | Periodicity |
 * __________________________________________________________________________________________________________________________
 * |  1     | Output a (digital) watchdog waveform (with same length and period of the ‘Normal’ |     50Hz    |   20ms      |
 * |        | operation of SigB in Assignment 1). Timings should be within 5%.                  |             |             |
 * __________________________________________________________________________________________________________________________
 * |  2     | Monitor one digital input (to be connected to a pushbutton/switch or a signal     |     5Hz     |   200ms     |
 * |        | generator for students using Proteus).                                            |             |             |
 * ____\______________________________________________________________________________________________________________________
 * |  3     | Measure the frequency of a 3.3v square wave signal. The frequency will be in the  |     1Hz     |   1000ms    |
 * |        | range 500Hz to 1000Hz and the signal will be a standard square wave (50% duty     |             |             |
 * |        | cycle). Accuracy to 2.5% is acceptable.                                           |             |             |
 * __________________________________________________________________________________________________________________________
 * |  4     | Read one analogue input. The analogue input must be connected to a maximum of     |     24Hz    |   42ms      |
 * |        | 3.3V, using a potentiometer.                                                      |             |             |
 * __________________________________________________________________________________________________________________________
 * |  5     | Compute filtered analogue value, by averaging the last 4 readings.                |     24Hz    |   42ms      |
 * |        |                                                                                   |             |             |
 * __________________________________________________________________________________________________________________________
 * |  6     | Execute 1000 times the following instruction:                                     |     10Hz    |   100ms     |
 * |        | __asm__ __volatile__ ("nop");                                                     |             |             |
 * |        | The statement could be repeated using a single loop, or broken down into multiple |             |             |
 * |        | loops (e.g. to be executed in different slots of the cyclic executive).           |             |             |
 * __________________________________________________________________________________________________________________________
 * |  7     | Perform the following check:if (average_analogue_in > half of maximum range for   |     3Hz     |   333ms     |
 * |        | analogue input):                                                                  |             |             |
 * |        |     error_code = 1                                                                |             |             |
 * |        | else:                                                                             |             |             |
 * |        |     error_code = 0                                                                |             |             |
 * __________________________________________________________________________________________________________________________
 * |  8     | Visualise error_code using an LED.                                                |     3Hz     |   333ms     |
 * |        |                                                                                   |             |             |
 * __________________________________________________________________________________________________________________________
 * |  9     | Check is the push button is currently in it's pressed state, if it is log the     |     0.3Hz   |   5000ms    |
 * |        | following information every five (5) seconds (in comma separated                  |             |             |
 * |        | format, e.g. CSV) to the serial port:                                             |             |             |
 * |        |   - State of the digital input (pushbutton / switch);                             |             |             |
 * |        |   - Frequency value (Hz, as an integer);                                          |             |             |
 * |        |   - Filtered analogue input.                                                      |             |             |
 * __________________________________________________________________________________________________________________________
 */


// determine the number of cores availible
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// pin assignments
#define LED 14          // Pin G14, Used to output error signal
#define WD 19           // Pin G19, Used to output the watchdog signal
#define PB1 12          // Pin G12, Used to read the state of a push button
#define A_IN 4          // Pin G4,  Used to read an analogue input signal
#define PULSE_IN 18     // Pin G18, Used to read a digital input signal

// Rate of task (ms)
/* while hz to ms gives 20.4, this 
* has been rounded to 42 as the ESP32's
* freeRTOS tick rate is 1ms
*/
#define R_T1 20         // 50 Hz
#define R_T2 200        // 5 Hz
#define R_T3 1000       // 1 Hz
/* while hz to ms gives 41.666666...this 
* has been rounded to 42 as the ESP32's
* freeRTOS tick rate is 1ms
*/
#define R_T4 42         // 24 Hz
#define R_T5 42         // 24 Hz
#define R_T6 100        // 10 Hz
/* while hz to ms gives 333.33333... this 
* has been rounded to 333 as the ESP32's
* freeRTOS tick rate is 1ms
*/
#define R_T7 333        // 3 Hz
#define R_T8 333        // 3 Hz
#define R_T9 5000       // 0.2 Hz

// alternate to vtaskdelay that allows the delay to 
// be defined using milliseconds rather than ticks
#define TaskDelay(x) vTaskDelay(x/portTICK_PERIOD_MS)

// prototype functions for tasks
void task_1(void *pvParameters);
void task_2(void *pvParameters);
void task_3(void *pvParameters);
void task_4(void *pvParameters);
void task_5(void *pvParameters);
void task_6(void *pvParameters);
void task_7(void *pvParameters);
void task_8(void *pvParameters);
void task_9(void *pvParameters);

// Defined Queue handles for analog signal
// and avaraged analogue signal queues
static QueueHandle_t analog_queue;
static QueueHandle_t average_queue;

// Mutex to protect the "data" struct variable
static SemaphoreHandle_t data_mut;

// error code variable for low analogue voltage check
volatile int error_code;

// Struct to collect all data to be output in task 9
struct Data
{
    bool button = false;
    int frequency = 0;
    float analog = 0;
} data;

// generate pulse of with 50us
void task_1(void *pvParameters)
{
    (void) pvParameters;
    for (;;)
    {
        digitalWrite(WD, HIGH);
        // 50 microsecond delay
        delayMicroseconds(50);
        digitalWrite(WD, LOW);

        // delays task for rate
        // before restarting
        TaskDelay(R_T1);
    }
}

// read input of a button on pin PB1
void task_2(void *pvParameters)
{   
    (void) pvParameters;
    for (;;)
    {
        // Mutex protection statement
        // prevents memory access issues
        // attempts to take the mutex access token
        // only continues if the access token is availible
        if(xSemaphoreTake(data_mut, portMAX_DELAY) == pdTRUE){
            data.button = digitalRead(PB1);

            // return mutex access token
            xSemaphoreGive(data_mut);
        }

        // delays task for rate
        // before restarting
        TaskDelay(R_T2);
    }
}

// determine frequency of digital signal on pin PULSE_IN
void task_3(void *pvParameters)
{   
    float duration = 0;
    
    (void) pvParameters;
    for (;;)
    {
        // Mutex protection statement
        // prevents memory access issues
        // attempts to take the mutex access token
        // only continues if the access token is availible
        if(xSemaphoreTake(data_mut, portMAX_DELAY) == pdTRUE){
            duration = pulseIn(PULSE_IN, LOW);
            data.frequency = 1000000.0 / (duration * 2);

            // return mutex access token
            xSemaphoreGive(data_mut);
        }

        // delays task for rate
        // before restarting
        TaskDelay(R_T3);
    }
}

// read analogue input on pin A_IN
void task_4(void *pvParameters)
{   
    int x = 0;
    
    (void) pvParameters;
    for (;;)
    {
        x = analogRead(A_IN);
        xQueueSend(analog_queue, &x, 100);

        // delays task for rate
        // before restarting
        TaskDelay(R_T4);
    }
}

// Average last 4 analog input readings
void task_5(void *pvParameters)
{ 
    float analogs[4] = {0, 0, 0, 0};    

    (void) pvParameters;
    for (;;)
    {
        int x = 0;
        float y = 0;

        if(xQueueReceive(analog_queue, &x, 100)){
            for (int i = 1; i < 4; i++)
            {
                analogs[i - 1] = analogs[i];
            }

            analogs[3] = x * (3.3 / 4095);
        }

        // Mutex protection statement
        // prevents memory access issues
        // attempts to take the mutex access token
        // only continues if the access token is availible
        if(xSemaphoreTake(data_mut, portMAX_DELAY) == pdTRUE){
            
            for (int i = 0; i < 4; i++)
            {
                y += analogs[i];
            }

            y = y / 4;

            data.analog = y;

            xQueueSend(average_queue, &y, 100);

            // return mutex access token
            xSemaphoreGive(data_mut);
        }

        // delays task for rate
        // before restarting
        TaskDelay(R_T5);
    }
}

// use "__asm__ __volatile__ ("nop");" 1000 times
void task_6(void *pvParameters)
{    
    (void) pvParameters;
    for (;;)
    {
        for (int i = 0; i < 1000; i++)
        {
            __asm__ __volatile__("nop");
        }

        // delays task for rate
        // before restarting
        TaskDelay(R_T6);
    }
}

// determine error code based on average analogue reading
void task_7(void *pvParameters)
{    
    (void) pvParameters;
    for (;;)
    {
        float x = 0;

        // tasks next item from average queue and assign is to variable x
        if (xQueueReceive(average_queue, &x, 100)){
            if ( x > (3.3 / 2))
            {
                error_code = 1;
            }
            else
            {
                error_code = 0;
            }
        }

        // delays task for rate
        // before restarting
        TaskDelay(R_T7);
    }
}

// light LED based on error code
void task_8(void *pvParameters)
{    
    (void) pvParameters;
    for (;;)
    {
        digitalWrite(LED, error_code);
    
        // delays task for rate
        // before restarting
        TaskDelay(R_T8);
    }
}

// print; button PB1 state, Frequency of PULSE_IN, and average of alalogue input A_IN
// This data is presented in a CSV format
void task_9(void *pvParameters)
{    
    (void) pvParameters;
    for (;;)
    {
        // Mutex protection statement
        // prevents memory access issues
        // attempts to take the mutex access token
        // only continues if the access token is availible
        if(xSemaphoreTake(data_mut, portMAX_DELAY) == pdTRUE){
            if (data.button == 1)
            {
                Serial.print(data.button);
                Serial.print(", \t\t\t");
                Serial.print(data.frequency);
                Serial.print(", \t\t");
                Serial.print(data.analog);
                Serial.print("\n");
            }

            // return mutex access token
            xSemaphoreGive(data_mut);
        }

        // delays task for rate
        // before restarting
        TaskDelay(R_T9);
    }
}

void setup()
{
    Serial.begin(115200);

    // setup pins
    pinMode(LED, OUTPUT);
    pinMode(WD, OUTPUT);
    pinMode(PB1, INPUT);
    pinMode(A_IN, INPUT);
    pinMode(PULSE_IN, INPUT);

    // header to assist with reading the CSV formatted output
    Serial.println("-------------------------------------");
    Serial.println("Switch, \tFrequency, \tInput");
    Serial.println("-------------------------------------");

    //setup mutex in order to protect the "data" struct
    data_mut = xSemaphoreCreateMutex();

    //setup queues for analog readings and averaged analogue readings
    analog_queue = xQueueCreate(1, sizeof(float));
    average_queue = xQueueCreate(1, sizeof(float));

    // setup freeRTOS tasks for each of the task functions
    /* xTaskCreate(
    *       task function,
    *       name for debugging purposes,
    *       stack size,
    *       pvParamer
    */

    xTaskCreate(
        task_1,
        "task 1",
        512,
        NULL,
        4,
        NULL);

    xTaskCreate(
        task_2,
        "task 2",
        512,
        NULL,
        3,
        NULL);

    xTaskCreate(
        task_3,
        "task 3",
        1024,
        NULL,
        3,
        NULL);

    xTaskCreate(
        task_4,
        "task 4",
        1024,
        NULL,
        2,
        NULL);

    xTaskCreate(
        task_5,
        "task 5",
        1024,
        NULL,
        1,
        NULL);

    xTaskCreate(
        task_6,
        "task 6",
        512,
        NULL,
        3,
        NULL);

    xTaskCreate(
        task_7,
        "task 7",
        1024,
        NULL,
        4,
        NULL);

    xTaskCreate(
        task_8,
        "task 8",
        512,
        NULL,
        3,
        NULL);

    xTaskCreate(
        task_9,
        "task 9",
        1024,
        NULL,
        4,
        NULL);
}

void loop(){}
