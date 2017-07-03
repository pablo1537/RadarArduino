#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>
#include "rgb_lcd.h" //libreira incluida aparte, para el manejo del display

//Macros
#define PIN_BOTON 3
#define PIN_LED 5
#define BT_RX 11
#define BT_TX 10
#define BAUD_SERIAL  9600
#define BAUD_BT 9600
#define PIN_POT A0
#define QUIETO 48
#define IZQ 49
#define DER 50
#define MODO_ANDROID 51
#define AUTO 52
#define COLOR 56

/*#define IZQ 1
#define DER 2
#define MODO_ANDROID 3
#define AUTO 4*/

//Declaraciones variables globales
//Bluetoth
SoftwareSerial BT(BT_RX ,BT_TX); // RX, TX crusados     //Hace un puerto serial virtual que seusara para el bluetoth, y dejar libre el normal para comuniccion con la PC
int flag=AUTO;
int dir=QUIETO;
int recivi=0;
//-----------------------------------------------------------------------------------------------------
//Variable que manejara el display
rgb_lcd lcd; 
int color=0;
//-----------------------------------------------------------------------------------------------------
//Ultrasonico
// Defines Trig and Echo pins of the Ultrasonic Sensor
const int trigPin = 8;
const int echoPin = 9;
// Variables for the duration and the distance
long  duration;
int distance;
unsigned Dist;

//-----------------------------------------------------------------------------------------------------
//Servo
Servo myServo;
int servo_pos=90;//angulo default
int servo_rota =0; //flag
int enPosicion=2;
int pos_mayor=155;
int pos_menor=25;
int pos;

//-----------------------------------------------------------------------------------------------------
//Potenciometro
//int potpin=A0;//pin del potenciometro
int potval;//valor del potenciometro
//-----------------------------------------------------------------------------------------------------
//Boton
// acordate de sacarlo int pin_boton = 3;// pin del boton 
int boton_estado =0; //estado del boton

void setup() {
 
Serial.begin(9600);
myServo.attach(2);//Seteo el servo en el pin 2
pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
pinMode(echoPin, INPUT); // Sets the echoPin as an Input
pinMode(PIN_BOTON,INPUT);//seteo el boton 
pinMode(PIN_LED,OUTPUT);//seteo el led
pinMode(BT_RX, INPUT);
pinMode(BT_TX, OUTPUT);//seteo el la comunicacion serial con el HC-05
delay(500);
BT.begin(9600); 

lcd.begin(16,2);//16 columans, 2 filas
lcd.setRGB(100,100,100);
lcd.display();



}
/*
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  /*pinMode(BT_RX, INPUT);
  pinMode(BT_TX, OUTPUT);
  pinMode(8,OUTPUT);
  delay(500);
   digitalWrite (8, HIGH);
   Serial.begin(BAUD_SERIAL);
  BT.begin(BAUD_BT);
  
  // BT.begin(115200);
  Serial.write("comienso");
*/

void loop() {
  if(BT.available())
              {
              recivi = BT.read();
              Serial.println(recivi);
              if(recivi == COLOR)
             { cambiarcolor();}
             else if(recivi == AUTO || recivi == MODO_ANDROID)
                       { flag=recivi;}
              else if(recivi ==QUIETO || recivi == IZQ || recivi == DER)         
                       { dir=recivi;}
                
              };
  if( enPosicion != 2)
  {
    posicionar();
    
  }else{  
    if(flag == AUTO)
    radar_auto();
    else
    radar_android();  
  }
  pantalla();
  Dist=calculateDistance();
  
   
    BT.print(Dist);
    BT.print(":");
    BT.print(servo_pos);
    BT.println(":");
   
  //reinicio
 if(digitalRead(PIN_BOTON)==HIGH)
   {
    enPosicion=0;
    delay(1000);
    servo_pos=pos_mayor;
   } 
    
  delay(150);
  
}

void posicionar() //Funcion que hubicara el barrido del servo
{
  //delay(1000);
  int aux=0;
  
     while(digitalRead(PIN_BOTON)!=HIGH)
        {
          potval = analogRead(PIN_POT); //Aqui le decimos que lea el valor del potenciometro, valor el cual oscila entre 0 y 1023
          potval = map(potval, 0 ,1023, 25, 155);//cambia el rango de valores del potenciometro a el rango q vamos a usar
          servo_pos=potval;
          myServo.write(servo_pos);
          delay(100);
        }
     if(enPosicion == 0)   
        {
        pos_mayor=servo_pos; 
        Serial.println("Se guardo variable 1");
        enPosicion=1;
        delay(1000);
        }else
        {
            pos_menor=servo_pos; //guardo el segundo anngulo
            if(pos_menor > pos_mayor)
                {
                aux=pos_menor;
                pos_menor=pos_mayor;
                pos_mayor=aux; //ajusto para que el menor sea el menor
                }
             Serial.println("Se guardo variable 2");   
             enPosicion=2;
             delay(1000);    
        }
     
}

void radar_auto()
{
   // Serial.print("entra_sweep     ");
   

    if(servo_rota==0)
        {
       // if(servo_pos<=(pos_mayor - potval))
        if(servo_pos<=(pos_mayor ))
            {
            servo_pos+=3;
            }else{
            servo_rota=1;
            }
        }else{
        //if(servo_pos>=(pos_menor + potval))
        if(servo_pos>=(pos_menor ))
            {
            servo_pos-=3;
            }else{
            servo_rota=0;
            }
        }
    myServo.write(servo_pos);
   // Serial.println(servo_pos);
}

void radar_android()
{
 // Serial.print("entra_modo_android     ");
  if(dir==IZQ)
        {
        if(servo_pos<=(pos_mayor ))
            {
            servo_pos+=2;
            }
        }
  if(dir==DER)
        {        
        if(servo_pos>=(pos_menor ))
            {
            servo_pos-=2;
            }
        }
    myServo.write(servo_pos);
   // Serial.println(servo_pos);
    
}

void pantalla()
{
  Dist=calculateDistance();
  if(Dist <=20 && Dist >= 1 )
    digitalWrite(PIN_LED, HIGH);
   else
    digitalWrite(PIN_LED, LOW);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distancia: ");
  lcd.print(Dist);
  lcd.setCursor(0, 1);
  lcd.print("Angulo: ");
  lcd.print(servo_pos);
}

int calculateDistance(){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(4);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
    distance= unsigned (int (duration* 0.017));
   // Serial.println(distance);
    return distance;
}

void cambiarcolor()
{
  if(color==0)
  {
    lcd.setRGB(000,200,000);
    color=1;
  }else if(color==1){
    lcd.setRGB(000,200,000);
    color=2;
  }else if(color==2){
    lcd.setRGB(200,000,000);
    color=3;
  }else if(color==3){
    lcd.setRGB(200,000,000);
    color=4;
  }else if(color==4){
    lcd.setRGB(000,000,200);
    color=5;
  }else if(color==5){
    lcd.setRGB(000,000,200);
    color=0;
  }
}
