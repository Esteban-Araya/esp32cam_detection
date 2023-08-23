#include "Motor_esp.h"
#include <Arduino.h>

enum Motor {DER,IZQ};

Motor_esp::Motor_esp(
    int canal_der_f,
    int canal_der_b,
    int canal_izq_f,
    int canal_izq_b,
    int motor_der_f,
    int motor_der_b,
    int motor_izq_f,
    int motor_izq_b,
    int frecuencia,
    int resolucion){

        this->canal_der_b = canal_der_b;
        this->canal_izq_f = canal_izq_f;
        this->canal_izq_b = canal_izq_b;
        this->motor_der_f = motor_der_f;
        this->motor_der_b = motor_der_b;
        this->motor_izq_f = motor_izq_f;
        this->motor_izq_b = motor_izq_b;
        this->frecuencia = frecuencia;
        this->resolucion = resolucion;

        initMotor();
}

Motor_esp::~Motor_esp()
{
}

void Motor_esp::initMotor(){

  ledcSetup(canal_der_f,frecuencia,resolucion);
  ledcSetup(canal_der_b,frecuencia,resolucion);
  ledcSetup(canal_izq_f,frecuencia,resolucion);
  ledcSetup(canal_izq_b,frecuencia,resolucion);

  ledcAttachPin(motor_der_f,canal_der_f);  
  ledcAttachPin(motor_der_b,canal_der_b);
  ledcAttachPin(motor_izq_f,canal_izq_f);  
  ledcAttachPin(motor_izq_b,canal_izq_b);
}

void Motor_esp::mover_adelante(int lado, int vel){


  if(lado == DER){
    ledcWrite(motor_der_f,vel);
    ledcWrite(motor_der_b,0);
  }
  else{
   ledcWrite(motor_izq_f,vel);
   ledcWrite(motor_izq_b,0);
  }
}
void Motor_esp::mover_atras(int lado, int vel){


 if(lado == DER){
    ledcWrite(motor_der_f,0);
    ledcWrite(motor_der_b,vel);
  }
  else{
   ledcWrite(motor_izq_f,0);
   ledcWrite(motor_izq_b,vel);
  }
}

void Motor_esp::avanzar(int vel){
  mover_adelante(DER,vel);
  mover_adelante(IZQ,vel);
}


void Motor_esp::doblar_derecha(int vel, int tiempo = 0){
  mover_adelante(IZQ,vel);
  mover_atras(DER,vel);
  delay(tiempo);
}

void Motor_esp::doblar_izquierda(int vel, int tiempo = 0){
  mover_adelante(DER,vel);
  mover_atras(IZQ,vel);
  delay(tiempo);
}

void Motor_esp::retroceder(int vel){
  mover_atras(IZQ,vel);
  mover_atras(DER,vel);
}

void Motor_esp::stop(){
  mover_adelante(IZQ,0);
  mover_atras(DER,0);

}