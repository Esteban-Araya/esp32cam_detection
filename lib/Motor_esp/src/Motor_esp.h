#ifndef Motor_esp_H
#define Motor_esp_H

class Motor_esp
{
private:
    int frecuencia;
    int resolucion;
    int canal_der_f;
    int canal_der_b;
    int canal_izq_f;
    int canal_izq_b;
    int motor_der_f;
    int motor_der_b;
    int motor_izq_f;    
    int motor_izq_b;        
    void initMotor();
    void mover_adelante(int lado, int vel);
    void mover_atras(int lado, int vel);
public:
    Motor_esp(
        int canal_der_f,
        int canal_der_b,
        int canal_izq_f,
        int canal_izq_b,
        int motor_der_f,
        int motor_der_b,
        int motor_izq_f,
        int motor_izq_b,
        int frecuencia,
        int resolucion
        );
    ~Motor_esp();
    void avanzar(int vel);
    void doblar_derecha(int vel,int tiempo = 0);
    void doblar_izquierda(int vel,int tiempo = 0);
    void retroceder(int vel);
    void stop();
};

#endif // Motor_esp_H