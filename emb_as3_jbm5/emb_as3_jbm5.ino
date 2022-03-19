/* Embbeded Software - Assignment 3
 * This project was programmed for for the ESP32 microcontroller, it uses freeRTOS to run a series of tasks based on the provided brief.
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
 * |  1     | Output a (digital) watchdog waveform (with same length and period of the ‘Normal’ | each cycle  |   1ms       | 
 * |        | operation of SigB in Assignment 1). Timings should be within 5%.                  |             |             |
 * __________________________________________________________________________________________________________________________
 * |  2     | Monitor one digital input (to be connected to a pushbutton/switch or a signal     |     5Hz     |   200ms     | 
 * |        | generator for students using Proteus).                                            |             |             |
 * __________________________________________________________________________________________________________________________
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

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// pin assignments
#define LED         14    // Pin G14, Used to output error signal
#define WD          19    // Pin G19, Used to output the watchdog signal
#define PB1         12    // Pin G12, Used to read the state of a push button
#define A_IN        2     // Pin G0,  Used to read an analogue input signal
#define PULSE_IN    18    // Pin G18, Used to read a digital input signal

// using lowest common multiple of the frequencies, a timing to allow all 
// tasks to be triggered was calculated, 8.33333333...ms
// to decrease the loss through rounding, this was then halved to 4.166...
#define T_CLK       4

// Rate of task (ticks)
#define R_T2        50        // 200  /4
#define R_T3        250       // 1000 /4
// while hz to ms gives 41.666666... for multiple
// reasons this has been rounded to 42
#define R_T4        10        // 41.666../4
#define R_T5        10        // 41.666../4
#define R_T6        25        // 100  /4
// while hz to ms gives 333.33333... for multiple
// reasons this has been rounded to 333
#define R_T7        83        // 333.33.../4
#define R_T8        83        // 333.33.../4
#define R_T9        1250      // 5000/4

struct data
{
    bool    button;
    int     frequency;
    float   analog;
};



void setup()
{
    Serial.begin(57600);
    // xTaskCreate
}

void loop()
{
    
}
