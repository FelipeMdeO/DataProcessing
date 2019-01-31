/*
  ADXL362_SimpleRead.ino -  Simple XYZ axis reading example
  for Analog Devices ADXL362 - Micropower 3-axis accelerometer
  go to http://www.analog.com/ADXL362 for datasheet


  License: CC BY-SA 3.0: Creative Commons Share-alike 3.0. Feel free
  to use and abuse this code however you'd like. If you find it useful
  please attribute, and SHARE-ALIKE!

  Created June 2012
  by Anne Mahaffey - hosted on http://annem.github.com/ADXL362

  Modified May 2013
  by Jonathan Ruiz de Garibay

  Connect SCLK, MISO, MOSI, and CSB of ADXL362 to
  SCLK, MISO, MOSI, and DP 10 of Arduino
  (check http://arduino.cc/en/Reference/SPI for details)

*/

#include <SPI.h>
#include <ADXL362.h>
#include <math.h>

#define LED_PIN 13
#define MOVE_INDICATOR 4
#define N_MEAN_SAMPLES 8
#define N_SAMPLES_TH 2

//#define DEBUG_PRINT

ADXL362 xl;

int16_t temp;
int16_t XValue, YValue, ZValue, Temperature;
double fx_mean;
double x_f[2];
int16_t x_mean[8];
unsigned int count;
int count_threshould;
long int x_mean_filtered;
int long acel_threshould;
bool start;

void setup() {

  //Serial.begin(38400);
  xl.begin(10);                   // Setup SPI protocol, issue device soft reset
  xl.setupOdr();
  xl.beginMeasure();              // Switch ADXL362 to measure mode
  xl.setupNoiseReject();

  fx_mean = 0;

  x_f[0] = 0;
  x_f[1] = 0;

  x_f[0] = 0;
  x_f[1] = 0;
  x_f[2] = 0;
  x_f[3] = 0;
  x_f[4] = 0;
  x_f[5] = 0;
  x_f[6] = 0;
  x_f[7] = 0;

  count = 0;
  count_threshould = 0;
  x_mean_filtered = 0;

  pinMode(LED_PIN, OUTPUT);
  pinMode(MOVE_INDICATOR, OUTPUT);
  digitalWrite(MOVE_INDICATOR, LOW);

#ifdef DEBUG_PRINT
  Serial.println("Finish setup.");
#endif

  Calibration();
}

void(* resetFunc) (void) = 0;//declare reset function at address 0

void loop() {

  if (start) 
  {
   while (!xl.getStatusDataReady());
   xl.readXData(); 
   while (!xl.getStatusDataReady());
   xl.readXData();
   while (!xl.getStatusDataReady());
   xl.readXData();

   start = false;
  }
  // read all three axis in burst to ensure all measurements correspond to same sample time
  while (!xl.getStatusDataReady()); // Aguardo uma amostra valida
  XValue = xl.readXData() - fx_mean;

#ifdef DEBUG_PRINT
  Serial.print("XValue");Serial.print("\t");Serial.println(XValue);
#endif

  if (count < N_MEAN_SAMPLES) 
  {
    x_mean[count] = XValue;
    count++;
  } 
  else 
  {
    count = 0;
    x_mean_filtered = round(average(x_mean, N_MEAN_SAMPLES));
    #ifdef DEBUG_PRINT
      Serial.print("x_mean_filtered");Serial.print("\t");Serial.println(x_mean_filtered);
    #endif
    
    if ((x_mean_filtered >= acel_threshould) || (x_mean_filtered < -acel_threshould))
    {
      count_threshould++;
      #ifdef DEBUG_PRINT
        Serial.println(count_threshould);
      #endif
      
    }
    else 
    {
      count_threshould = 0;
    }

    if (count_threshould >= N_SAMPLES_TH) {
      move_identification();
      //reset
    }
  }
}

  void move_identification() {
    //Serial.println("Move detected! Plx Restart Process"); 
    digitalWrite(MOVE_INDICATOR, HIGH);
    delay(3000);
    digitalWrite(MOVE_INDICATOR, LOW);
    resetFunc(); //call reset
  }

  // todo, verificar o que pode ser feito para reduzir o tempo de execução desta funcao

  void Calibration(void) {
    int nSamples = 8;    //todo passa isso daqui para #define
    int i;// = 0;
    int ax[nSamples];
    fx_mean = 0;

    //NOTE First time, values can be junk
    while (!xl.getStatusDataReady());
    xl.readXData(); 
    while (!xl.getStatusDataReady());
    xl.readXData();
    while (!xl.getStatusDataReady());
    xl.readXData();
    while (!xl.getStatusDataReady());
    xl.readXData();

    for(i=0;i<8;i++) {
      while (!xl.getStatusDataReady()); // Aguardo uma amostra valida
      ax[i] = xl.readXData();
      #ifdef DEBUG_PRINT
        Serial.print("ax[");Serial.print(i); Serial.print("]"); Serial.print("\t"); Serial.println(ax[i]);
      #endif
    }
   
    fx_mean = average(ax, nSamples);
    acel_threshould = 125;

#ifdef DEBUG_PRINT
    Serial.print("Calibration Finish"); Serial.print("\t"); Serial.println(fx_mean);
#endif

    for (i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }


   start = true;

  }

  float average(int * array, int len) {

    #ifdef DEBUG_PRINT
      Serial.println("Average Calculation in process");
    #endif
    long sum = 0L;
    for (int i = 0; i < len; i++)
      sum += array[i];
    return ((float) sum) / len;
  }
