/*      Codigo capaz de ler o MPU6050 e a partir de filtros e do calculo da velocidade 
        interpretar se houve movimento ou não, ou seja, faz o cancelamento de ruídos e 
        vibracoes                                                                           */


#include "I2Cdev.h"
#include "MPU6050.h"

#define SAMPLE_TIME 10       // Tempo entre leituras de 10 ms
#define STOP_TIME 1000       // Tempo entre leituras de 1000 ms
#define DT_THRESHOULD 500
#define N_SAMPLES 40        // Numero de amostras para identificar que saiu da boca do tanque
#define ACEL_THRESHOULD 65 // Limiar de aceleracao em mg

#define LED_PIN 13
#define INTERRUPT_PIN 2
#define CALIBRATION_PIN 3

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MPU6050 accelgyro;

int16_t ax, ay, az;
unsigned int count = 0;
unsigned int count2 = 0;
unsigned int t_vector[2] = {0, 0};
unsigned int time_pass = 0;

double v_mean_y[7];
double ax2, ay2;
double fx_mean, fy_mean, fz_mean;
double y_f[2], x_f[2], vel_x[2], vel_y[2], pos_y[2];

int my_direction = 0;
bool can_calc_my_direction = true;
bool last_time_put = false;

unsigned short int count_threshould = 0;
unsigned long lastTime;
unsigned long tempo_inv_now;
unsigned long tempo_inv_last;
unsigned long timeChange;
unsigned long now;
unsigned long t_threshould = 0;

void setup() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
  Serial.begin(38400);      // abre a porta serial a 38400 bps:
  
  accelgyro.initialize();

  accelgyro.setDHPFMode(0b001); // Colocado aqui 5 Hz (talvez nao necessario)
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

  pinMode(LED_PIN, OUTPUT);
  pinMode(INTERRUPT_PIN, INPUT);
  pinMode(CALIBRATION_PIN, INPUT);

  digitalWrite(INTERRUPT_PIN, HIGH);
  digitalWrite(CALIBRATION_PIN, HIGH);

  y_f[0] = 0;
  y_f[1] = 0;
  x_f[0] = 0;
  x_f[1] = 0;
  vel_x[0] = 0;
  vel_x[1] = 0;
  vel_y[0] = 0;
  vel_y[1] = 0;
  pos_y[0] = 0;
  pos_y[1] = 0;
  v_mean_y[0] = 0;
  v_mean_y[1] = 0;
  v_mean_y[2] = 0;
  v_mean_y[3] = 0;
  v_mean_y[4] = 0;
  v_mean_y[5] = 0;
  v_mean_y[6] = 0;
  v_mean_y[7] = 0;
  count = 0;
  count2 = 0;

  Calibration();

  digitalWrite(LED_PIN, LOW);

  lastTime = 0;
}

void loop() {

  while (!digitalRead(INTERRUPT_PIN)) {
    now = millis();
    timeChange = (now - lastTime);

    if (timeChange >= SAMPLE_TIME) {

      count++;

      /* Update accel data */
      accelgyro.getAcceleration(&ax, &ay, &az);

      /* Filter x and y axis */
      /*
        for k=2:length(y)
        y(k) = 0.8819*y(k-1) + 0.1181*x(k-1);
        end
      */

      ay2 = double((ay - fy_mean) * 2.0 / 16783.0);
      ax2 = double((ax - fx_mean) * 2.0 / 16783.0);

      //XXX
      // para o teste neste momento sera utilizado o eixo X apenas
      ay2 = ax2;
      
      y_f[1] = 0.8819 * y_f[0] + 0.1181 * ay2;
      //y[0] = y[1];
      //x_f[1] = 0.8819 * x_f[0] + 0.1181 * ax2;
      //x[0] = x[1];

      if ((y_f[1] >= ACEL_THRESHOULD/1000.0) || (y_f[1] < -ACEL_THRESHOULD/1000.0)) {
        count_threshould++;
      } 
      else {
        count_threshould = 0;
      }

      if (count_threshould >= N_SAMPLES) {
        //Serial.println(d);
        move_identification();
        while(!digitalRead(INTERRUPT_PIN)); // Travo aqui ate que botao seja soltado para que processo seja reiniciado
      }
      /* Integral calculation */
      /*
         for i=1:length(x2)-1
           vel(i+1) = vel(i) + dt*(x2(i)+x2(i+1))/2;
         end
      */
      /*
      vel_y[1] = vel_y[0] + (0.01) * (y_f[1] + y_f[0]) / 2.0;
      //vel_x[1] = vel_x[0] + (0.1) * (x_f[1] + x_f[0]) / 2.0;

      /* Seconde Integral Calculation */
      /*
      pos_y[1] = pos_y[0] + (0.01)* (vel_y[1] + vel_y[0]) / 2.0;

      y_f[0] = y_f[1]; // Atualizacao dos valores de aceleracao em Y
      //x_f[0] = x_f[1]; // Atualizacao dos valores de aceleracao em X

      vel_y[0] = vel_y[1]; // Atualizacao dos valores de velocidade em Y
      //vel_x[0] = vel_x[1]; // Atualizacao dos valores de velocidade em X

      pos_y[0] = pos_y[1]; // Atualizacao dos valores de posicao em Y
      v_mean_y[7] = vel_y[1]; // Atribuicao de velocidade corrente ao vetor de media movel

      // so a partir da terceira iteracao faco a media movel
      if (count >= 8) {
        v_mean_y[7] = (v_mean_y[0] + v_mean_y[1] + v_mean_y[2] + v_mean_y[3]
                       + v_mean_y[4] + v_mean_y[5] + v_mean_y[6] + v_mean_y[7]) / 8.0;
        if (count2 <= 3) count2++;
      }

      v_mean_y[0] = v_mean_y[1];
      v_mean_y[1] = v_mean_y[2];
      v_mean_y[2] = v_mean_y[3];
      v_mean_y[3] = v_mean_y[4];
      v_mean_y[4] = v_mean_y[5];
      v_mean_y[5] = v_mean_y[6];
      v_mean_y[6] = v_mean_y[7];

      // Depois de ter duas atualizacoes dos valores de v_mean_y
      // Faco a verificacao de alternancia de maximos e minimos
      if (count2 >= 2) {
        if (can_calc_my_direction) {
          can_calc_my_direction = false;
          if ((v_mean_y[6] - v_mean_y[5]) > 0) {
            my_direction = 1;
          } else {
            my_direction = 0;
          }
        }
        if (my_direction) {                       // subindo
          if (v_mean_y[6] - v_mean_y[5] < 0.0) { // se passei a descer
            my_direction = 0;
            t_vector[1] = time_pass;
            if ((t_vector[1] - t_vector[0]) > DT_THRESHOULD) {
              move_identification();
            }
            t_vector[0] = t_vector[1];
          }
        }
        else {                                  //se estou descendo
          if (v_mean_y[6] - v_mean_y[5] > 0.0) {
            my_direction = 1;
            t_vector[1] = time_pass;
            if ((t_vector[1] - t_vector[0]) > DT_THRESHOULD) {
              move_identification();
            }
            t_vector[0] = t_vector[1];
          }
        }
      }
      */
      Serial.println(y_f[1]);// Serial.print("\t"); Serial.print(v_mean_y[6]);
      //Serial.print("\t"); Serial.print(vel_y[1]), Serial.print("\t"); Serial.println(pos_y[1]);      
      lastTime = millis();
      time_pass++; //atualizacao do tempo da iteracao
    }
  }

  if (!digitalRead(CALIBRATION_PIN)) { // Se pressionei o pino pra calibrar isso ocorre
    Calibration();
  }

  y_f[0] = 0;
  y_f[1] = 0;
  x_f[0] = 0;
  x_f[1] = 0;
  vel_x[0] = 0;
  vel_x[1] = 0;
  vel_y[0] = 0;
  vel_y[1] = 0;
  pos_y[0] = 0;
  pos_y[1] = 0;
  v_mean_y[0] = 0;
  v_mean_y[1] = 0;
  v_mean_y[2] = 0;
  v_mean_y[3] = 0;
  v_mean_y[4] = 0;
  v_mean_y[5] = 0;
  v_mean_y[6] = 0;
  v_mean_y[7] = 0;
  count = 0;
  count2 = 0;
  can_calc_my_direction = true;

  t_vector[0] = 0;
  t_vector[1] = 0;

  time_pass = 0;

  //Serial.print(0); Serial.print("\t"); Serial.println(0);
}



void Calibration() {
  int i = 0;
  fx_mean = 0;
  fy_mean = 0;
  fz_mean = 0;
  int nSamples = 100;
  while (i < nSamples) {
    accelgyro.getAcceleration(&ax, &ay, &az);

    //por hora trabalhar so com x e y
    fx_mean += ax;
    fy_mean += ay;
    fz_mean += az;
    i++;
    delay(10);
  }

  fx_mean = fx_mean / nSamples;
  fy_mean = fy_mean / nSamples;
  fz_mean = fz_mean / nSamples;

  for (i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

void move_identification() {
   Serial.println("Move detected! Plx Restart Process");
  /*digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);*/
}
