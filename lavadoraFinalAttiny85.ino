// Código para una lavadora china (Shein o Temu) para el Attiny85
//
bool estaEncendido = false;
//------------Area de programaciOn de la clase "Boton"------------------------------
/*
  Obtenido el 31/12/2024 de 
  https://forum.arduino.cc/t/como-no-leer-un-boton-y-como-si-debemos-hacerlo/627659/4 
  */

#define APRETADO    0
#define SUELTO      1
#define PB0 0
#define PB1 1
#define PB2 2
#define PB3 3
#define PB4 4

class BotonSimple {
  private:
    unsigned char pin;
    unsigned char anterior, valor;
    unsigned char estado;

    unsigned long temporizador;
    unsigned long tiempoRebote;
  public:
    BotonSimple(unsigned char _pin, unsigned long _tiempoRebote);
    void actualizar();
    int  leer();
};

BotonSimple::BotonSimple(unsigned char _pin, unsigned long _tiempoRebote=50) {
  pin = _pin;
  tiempoRebote = _tiempoRebote;
  pinMode(pin, INPUT_PULLUP);
  valor=HIGH; anterior=HIGH; estado=SUELTO;
}

void BotonSimple::actualizar() {
  // NOTA: En el ejemplo original en vez de "pin" leia directamente el pin "2", con lo que el
  // codigo no funciona correctamente con el pin que le hayamos asignado.
  if ( valor==digitalRead(pin) ){
    temporizador=0;
  }
  else
  if ( temporizador==0 ) {
    temporizador = millis();
  }
  else
  if ( millis()-temporizador >= tiempoRebote ) {
    valor = !valor;
  }
  if ( anterior==LOW  && valor==LOW  ) estado = APRETADO;
  if ( anterior==HIGH && valor==HIGH ) estado = SUELTO;
  anterior = valor;
}

int BotonSimple::leer() { 
  return estado;
}

BotonSimple boton(PB0);

uint8_t accion(){
  boton.actualizar();
  switch ( boton.leer() ) {
    case SUELTO: return 1; break;
    case APRETADO: return 3; break;
    default: return 0; break;
  }
}
//-----------Fin del Area de clase "Boton"----------------------------------------- 
  



//-----------------FunciOn MenU------------------------------
unsigned long menu(){
  unsigned long inicio = millis() + 4000UL;
  uint8_t tiempoDeLavado[] = {5, 10, 3, 0};
  uint8_t i = 0;
  bool listo = false;
  
  while(millis() < inicio){
    if(i == 4){ 
        i = 0;
    }
         
    if(i == 0 && !listo){ // 5 minutos
      digitalWrite(PB1, 1);
      digitalWrite(PB2, 0);
      listo = true;

    }
    else if(i == 1 && !listo){ // 10 minutos
      digitalWrite(PB1, 0);
      digitalWrite(PB2, 1);
      listo = true;
    }
    else if(i == 2 && !listo){ // 3 minu
      digitalWrite(PB1, 1);
      digitalWrite(PB2, 1);
      listo = true;
    }
    else if(i == 3 && !listo){ // cancelar
      digitalWrite(PB1, 0);
      digitalWrite(PB2, 0);
      listo = true;
    }
     
    if(accion() == 3){
      while(accion() == 3);
      i++;
      inicio = millis() + 4000UL;
      listo = false;
    }
  }
  return tiempoDeLavado[i];
}//Fin de la funciOn menU
//------------------Funcion Trabajando-----------------------


//-------------Funcion Cancelar-----------------------------
void cancelar(){
  digitalWrite(PB3, 0);
  digitalWrite(PB4, 0);
  digitalWrite(PB1, 0);
  digitalWrite(PB2, 0);
  estaEncendido = false;
}           
//----------Fin FunciOn cancelar----------------------------


//------------Funcion Trabajando----------------------------
void trabajando(unsigned long t){
  t = t * 60000UL;
  unsigned long inicio = millis() + t;
  unsigned long tAlto;
  unsigned long tBajo;
  bool detener = false;

  while(t > millis()){
    digitalWrite(PB3, 1);
    digitalWrite(PB4, 0);
    
    tAlto = millis() + 9000UL;
    while(tAlto > millis()){
      if(accion() == 3){
        cancelar();
        detener = true; 
        break;
      }
    }
    
    if(detener){break;}
    
    digitalWrite(PB3, 0);
    digitalWrite(PB4, 0);
    tBajo = millis() + 2000UL;

    while(tBajo > millis()){
      if(accion() == 3){
        cancelar();
        detener = true;
        break;
      }
    }
    if(detener){break;}
    
    digitalWrite(PB3, 0);
    digitalWrite(PB4, 1);
    tAlto = millis() + 9000UL;
    while(tAlto > millis()){
      if(accion() == 3){
        cancelar();
        detener = true;
        break;
      }
    }
    if(detener){break;}
    
    digitalWrite(PB3, 0);
    digitalWrite(PB4, 0);
    tBajo = millis() + 2000UL;
    while(tBajo > millis()){
      if(accion() == 3){
        while(accion() == 3);
        cancelar();
        detener = true;
        break;
      }
    }
    if(detener){break;}
  }
  
  digitalWrite(PB1, 0);
  digitalWrite(PB2, 0);
  digitalWrite(PB3, 0);
  digitalWrite(PB4, 0);
  return;
}
//-------------Fin FunciOn Trabajando------------------------

void(* resetSoftware)(void) = 0;

//------------------Setup-----------------------------------
void setup(){ 
 // Estos pins son de señalización del menU con LEDS
 pinMode(PB1, OUTPUT);
 pinMode(PB2, OUTPUT); 
 
 // Estos PINS controlan al motor. 
 pinMode(PB3, OUTPUT);
 pinMode(PB4, OUTPUT); 
}

//---------------------Fin de Setup------------------------


//----------------------Loop-------------------------------
void loop(){
  unsigned long opcion;

  if(accion() == 3){
    while(accion() == 3);
    estaEncendido = true;
  }
  else if(accion() == 1){
    estaEncendido = false;

    if(millis() == 120000UL){resetSoftware();}
  }


  if(estaEncendido){//if(1)
    opcion = menu();
  }//Fin de if(1)
  
  //Aqui, la lavadora comienza a trabajar
  if(estaEncendido){
  	trabajando(opcion);
    estaEncendido = false;
    while(accion() == 3);
    resetSoftware();
  }
}
//--------------------Fin de loop----------------------------


