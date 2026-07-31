//*************************************************************************/
// Universidad del Valle de Guatemala
// BE3029 - Electrónica Digital 2
// Ana López - 241204
// 24/07/26
// Laboratorio 2: ISRs y Timers
//*************************************************************************/

#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <driver/gpio.h>

//===============================================================
// Definiciones     
//===============================================================
#define alarmaTimer 25000

// LEDs Contador Timer
#define ledContador1 13
#define ledContador2 12
#define ledContador3 14
#define ledContador4 15

// LEDs Contador Botón
#define ledBoton1 26
#define ledBoton2 25
#define ledBoton3 33  
#define ledBoton4 32

// LED Alarma
#define ledAlarma 2

// Botones
#define botonAumento 4
#define botonMenos 17

// Sensor capacitivo
#define sensorCapacitivo 27

//===============================================================
// Variables globales
//===============================================================
hw_timer_t * Timer0_Cfg = NULL;

volatile uint8_t contadorBinarioTimer = 0;
volatile uint8_t contadorBinarioBoton = 0;
volatile uint8_t estadoAlarmaActual = 0;
volatile uint8_t alarmaActivada = 0;
volatile uint32_t ultimoTiempoMas = 0;
volatile uint32_t ultimoTiempoMenos = 0;

volatile bool botonAumentoPresionado = false;
volatile bool botonMenosPresionado = false;

uint32_t ultimoTiempoBoton = 0;

const uint32_t debounceBoton = 100; 
const uint8_t umbralSensor = 40;

int LEDsTimer[] = {ledContador1, ledContador2, ledContador3, ledContador4};
int LEDsBoton[] = {ledBoton1, ledBoton2, ledBoton3, ledBoton4};

//===============================================================
// Prototipos
//===============================================================
void configTimer(void); 
void mostrarContadores(void);
void verificarAlarma(void);
void botonesPresionados(void);

void IRAM_ATTR Timer0_ISR(void);
void IRAM_ATTR ISRbotonAumento(void);
void IRAM_ATTR ISRbotonMenos(void);
void IRAM_ATTR ISR_sensorCapacitivo(void);

//===============================================================
// ISRs (Con debounce mínimo en ISR)
//===============================================================

void IRAM_ATTR Timer0_ISR(void) {
  // sucede cada 250 ms
  contadorBinarioTimer++;
  if (contadorBinarioTimer > 15) {
    contadorBinarioTimer = 0;
  }
}

void IRAM_ATTR ISRbotonAumento() {
  uint32_t tiempoActual = millis();
  if ((tiempoActual - ultimoTiempoMas) >= debounceBoton) {
    botonAumentoPresionado = true;
    ultimoTiempoMas = tiempoActual;
  }
}

void IRAM_ATTR ISRbotonMenos(void) {
  uint32_t tiempoActual = millis();
  if ((tiempoActual - ultimoTiempoMenos) >= debounceBoton) {
    botonMenosPresionado = true;
    ultimoTiempoMenos = tiempoActual;
  }
}


void IRAM_ATTR ISR_sensorCapacitivo(void) {
  contadorBinarioTimer = 0;
  timerWrite(Timer0_Cfg, 0);
}

//===============================================================
// SETUP
//===============================================================
void setup() {
  Serial.begin(115200);

  pinMode(ledContador1, OUTPUT);
  pinMode(ledContador2, OUTPUT);
  pinMode(ledContador3, OUTPUT);
  pinMode(ledContador4, OUTPUT);

  pinMode(ledBoton1, OUTPUT);
  pinMode(ledBoton2, OUTPUT);
  pinMode(ledBoton3, OUTPUT);
  pinMode(ledBoton4, OUTPUT);

  pinMode(botonAumento, INPUT_PULLDOWN);
  pinMode(botonMenos, INPUT_PULLDOWN);

  pinMode(ledAlarma, OUTPUT);
  digitalWrite(ledAlarma, LOW);

  configTimer();

  attachInterrupt(digitalPinToInterrupt(botonAumento), ISRbotonAumento, RISING);
  attachInterrupt(digitalPinToInterrupt(botonMenos), ISRbotonMenos, RISING);
  touchAttachInterrupt(sensorCapacitivo, ISR_sensorCapacitivo, umbralSensor);

  ultimoTiempoBoton = millis();
  ultimoTiempoMas = millis();
  ultimoTiempoMenos = millis();
}

//===============================================================
// LOOP 
//===============================================================
void loop() {
  botonesPresionados();
  mostrarContadores();
  verificarAlarma();

  // Para determinar umbral sensor capacitivo
  /*int32_t lecturaSensor = touchRead(sensorCapacitivo);
  Serial.print(lecturaSensor);
  delay(1000);*/
}

//===============================================================
// FUNCIONES AUXILIARES
//===============================================================

void configTimer(void) {
  Timer0_Cfg = timerBegin(0, 800, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, alarmaTimer, true);
  timerAlarmEnable(Timer0_Cfg);
}

void mostrarContadores(void) {
  // Mostrar contador del timer
  for(int j = 0; j < 4; j++) {
    if(((contadorBinarioTimer >> j) & 1) == 1) {
      digitalWrite(LEDsTimer[j], HIGH);
    } else {
      digitalWrite(LEDsTimer[j], LOW);
    }
  }

  // Mostrar contador del botón
  for(int i = 0; i < 4; i++) {
    if(((contadorBinarioBoton >> i) & 1) == 1) {
      digitalWrite(LEDsBoton[i], HIGH);
    } else {
      digitalWrite(LEDsBoton[i], LOW);
    }
  }
}

void verificarAlarma(void) {
  if(contadorBinarioBoton == contadorBinarioTimer) {
    if(alarmaActivada == 0) {
      alarmaActivada = 1;

      estadoAlarmaActual = !estadoAlarmaActual;
      digitalWrite(ledAlarma, estadoAlarmaActual);

      contadorBinarioTimer = 0;
      timerWrite(Timer0_Cfg, 0);
    } 
  } else {
    alarmaActivada = 0;
  }
}

void botonesPresionados(void) {
  if (botonAumentoPresionado) {
    contadorBinarioBoton++;
    if (contadorBinarioBoton > 15) {
      contadorBinarioBoton = 0;
    }
    botonAumentoPresionado = false;
  }
  
  if (botonMenosPresionado) {
    if (contadorBinarioBoton == 0) {

    contadorBinarioBoton = 15;
    } else
    contadorBinarioBoton--;
    botonMenosPresionado = false;
  }
}
