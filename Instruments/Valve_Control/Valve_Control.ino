#include <Wire.h>
#include<math.h>
#include <string.h>

const byte numChar=32;
char comm[numChar];
char p_comm[numChar];
int valve_num=0;
char valve_state[numChar]={0};
char valve_comm[numChar]={0};
const byte valNum=8;
int valSol_state[valNum]={0,0,0,0,0,0,0,0};
byte sol_PinMap[valNum]={4,7,8,9,10,11,12,13};
volatile bool errBuffer=false;
String errState;
int GateNum=3;
int VentNum=4;
int PulseNum=4;
const byte PG_SP3=A0;
const byte PG_SP4=A1;
const byte Turbo_SP=A2;
const byte VentPin=A3;
unsigned long pulse_time=1;
char pulse_prop[numChar]={0};


volatile byte interrupt=0;

//Interrupt 1
volatile byte INT_OUT=0;
volatile byte IO_1=3;
volatile byte InterruptValve1=5;

//Interrupt 2
volatile byte IO_2=2;
volatile byte InterruptValve2=3;

boolean NewData=false;
boolean PulseState=false;

int int1_stat_old=LOW;
int int2_stat_old=LOW;

void setup() {
  for (byte i=0; i<=valNum; i++){
    pinMode(sol_PinMap[i], OUTPUT);
  }
  set_valState();
  Serial.begin(115200);
  while(!Serial);
  pinMode(IO_1, INPUT); // setpoint 1 trigger  pin
  pinMode(IO_2, INPUT); //setpoint 2 trigger pin
  pinMode(PG_SP3, INPUT); //Pressure Gauge Relay 3
  pinMode(PG_SP4, INPUT); //Pressure Gauge Relay 4
  pinMode(Turbo_SP, INPUT); //Turbo Set  Point
  pinMode(VentPin, INPUT); //vent trigger

  int1_stat_old=digitalRead(IO_1);
  int2_stat_old=digitalRead(IO_2);
  
}

void processCommand(){
  if(strcmp(valve_comm,"SET")==0){
    if(valve_num>0 && valve_num<=8){
      if (strcmp(valve_state, "PULSE")==0){
        if(valve_num==PulseNum){
          pulse();
        }
        else{
          Serial.println("ERR:INV_VAL");
        }
      }
      else if(strcmp(valve_state,"OPEN")==0 || strcmp(valve_state,"CLOSE")==0){
        if(valve_num!=GateNum && valve_num!=VentNum){
          if(strcmp(valve_state,"OPEN")==0){
            if(valve_num!=InterruptValve1 && valve_num!=InterruptValve2){
             open_valve(); 
            }
            else if(interrupt==0){
              open_valve();
            }
          }
          else if(strcmp(valve_state,"CLOSE")==0){
            close_valve();
          }
         set_valState();
         Serial.print(valve_num);
         Serial.print(":");
         Serial.println(valSol_state[valve_num-1]);
        }
        else if(valve_num==GateNum){
          Gate_check();
          set_valState();
        }
        else if(valve_num==VentNum){
          Vent_Check();
          set_valState();
        }
      }
      else{
        Serial.println("ERR:STATE");
      }
    }
    else{
     Serial.println("ERR:NUM");
    }
  } 
  else if(strcmp(valve_comm,"GET")==0){
   if(valve_num<=8){
    if(valve_num==0){
      String out;
      out+=String(valve_num)+":[";
      for(byte i=0; i<valNum; i++){
        out+=String(valSol_state[i]);
         if(i<valNum-1){
          out+=",";
         }
         else{
          out+="]";     
         }
        }
        Serial.println(out);
     }
     else{
      Serial.println(String(valve_num)+":"+String(valSol_state[valve_num-1]));
     }
    }
    else{
      Serial.println("ERR:NUM");
    }
   }
  else if(strcmp(valve_comm, "PULSE")==0){
    if(strcmp(pulse_prop, "TIME")==0){
      Serial.println("TIME:"+String(pulse_time));
    }
    else if(strcmp(pulse_prop, "STATE")==0){
      Serial.println("STATE:"+String(PulseState));
    }
  }
 }

void close_valve(){
  if (PulseState==0){
    valSol_state[valve_num-1]=0;
  }  
}

void open_valve(){
  if (PulseState==0){
    valSol_state[valve_num-1]=1;
  }  
}


void set_valState(){
  for (byte i=0; i<=valNum; i++){
    digitalWrite(sol_PinMap[i], valSol_state[i]);
  }
}

void reset(){
  interrupt=0;
  Serial.println("RESET:SUCCESS");
}

void loop() {
  int int1_stat_new=digitalRead(IO_1);
  int int2_stat_new=digitalRead(IO_2);
  
  //Serial.println(String(int1_stat_new)+", "+String(int1_stat_old));
  Serial_Recv();
  if (int1_stat_new==LOW || int2_stat_new==LOW){
    if(int1_stat_new==LOW && int1_stat_old==HIGH && valSol_state[InterruptValve1-1]!=INT_OUT){
      valveClose1();
    }
    else if(int2_stat_new==LOW && int2_stat_old==HIGH && valSol_state[InterruptValve2-1]!=INT_OUT && digitalRead(Turbo_SP)==HIGH){
      valveClose2();
    }
  }
 int1_stat_old=int1_stat_new;
 int2_stat_old=int2_stat_new;
  if (NewData==true){
    if(errBuffer==false){
      strcpy(p_comm, comm);
      parseComm();
      processCommand();
      NewData=false;
    }
    else{
      Serial.println(String(errState));
      errBuffer=false;
      NewData=false;
    }
  }
 delay(5);
}

void Serial_Recv(){
  char rc;
  static byte ndx=0;
  static boolean recvInProgress = false;
  while(Serial.available() > 0 && NewData==false){
    rc = Serial.read();
    if(recvInProgress = true){;
      if( rc != '\n'){
        comm[ndx] = rc;
        ndx++;
      }
      else{
        comm[ndx]='\0';
        ndx=0;
        NewData=true;
      }
    }
    else if (rc!=0){
      recvInProgress=true;
    }
  }
}

void parseComm(){
  char *index;
  index=strtok(p_comm, ":");
  strcpy(valve_comm, index);
  if(strcmp(valve_comm, "SET")==0){
    index=strtok(NULL, ":");
    strcpy(valve_state, index);
    index=strtok(NULL, ":");
    valve_num=atoi(index);    
  }
  else if(strcmp(valve_comm,"GET")==0){
    index=strtok(NULL, ":");
    valve_num=atoi(index);
  }
  else if(strcmp(valve_comm,"PULSE")==0){
    index=strtok(NULL, ":");
    strcpy(pulse_prop, index);
    if(strcmp(pulse_prop,"TIME")==0){
      index=strtok(NULL, ":");
      pulse_time=atoi(index);
    }
    else if (strcmp(pulse_prop, "STATE")==0){
    }
    else{
      Serial.println("ERR:VAL");
      }
  }
  else if(strcmp(valve_comm,"RESET")==0){
    reset();
  }
  else{
    Serial.println("ERR:COMM");
  }
}

void valveClose1(){
  digitalWrite(sol_PinMap[InterruptValve1-1], INT_OUT);
  valSol_state[InterruptValve1-1]=INT_OUT;
  errBuffer=true;
  errState="ERR:INT1\0";
  interrupt=1;
}

void valveClose2(){
  digitalWrite(sol_PinMap[InterruptValve2-1], INT_OUT);
  valSol_state[InterruptValve2-1]=INT_OUT;
  errBuffer=true;
  errState="ERR:INT2\0";
  interrupt=1;
}

void Gate_check(){
  byte T_state=digitalRead(Turbo_SP);
  byte P_state=digitalRead(PG_SP3);
  if(strcmp(valve_state, "OPEN")==0){
    if(T_state==HIGH && P_state==LOW){
      Serial.println("INTLK:OVERPRESSURE");
    }
    else{
      if(GateNum!=InterruptValve1 && GateNum!=InterruptValve2){
        valve_num=GateNum;
        open_valve();
      }
      else if(interrupt==0){
        valve_num=GateNum;
        open_valve();
      }
      Serial.println(String(GateNum)+":"+String(valSol_state[GateNum-1]));
    }
  }
  else if(strcmp(valve_state, "CLOSE")==0){
    valve_num=GateNum;
    close_valve();
    Serial.println(String(GateNum)+":"+String(valSol_state[GateNum-1]));
  }
}

void Vent_Check(){
  byte vent_state=digitalRead(VentPin);
  if(strcmp(valve_state, "OPEN")==0){
    if(vent_state==HIGH){
      valve_num=VentNum;
      open_valve();
      Serial.println(String(VentNum)+":"+String(valSol_state[VentNum-1]));
    }
    else{
      Serial.println("INTLK:VENT");
    }
  }
  else if(strcmp(valve_state, "CLOSE")==0){
    valve_num=VentNum;
    open_valve();
    Serial.println(String(VentNum)+":"+String(valSol_state[VentNum-1]));
   }
 }

void pulse(){
  Serial.println("START:PULSE");
  PulseState=true;
  unsigned long start_time=millis();
  while(millis()-start_time<=8000){
    if (digitalRead(PG_SP4)==HIGH){
      digitalWrite(sol_PinMap[PulseNum-1], HIGH);
      delay(15);
      digitalWrite(sol_PinMap[PulseNum-1], LOW);
      delay(50);
    }
  }
  serial_flush();
  while(millis()-start_time<=(pulse_time*60000)-8000){
    digitalWrite(sol_PinMap[PulseNum-1], HIGH);
    Serial_Recv();
    if (NewData==true){
      strcpy(p_comm, comm);
      parseComm();
      processCommand();
      NewData=false;
    }
    delay(5);
  }
  digitalWrite(sol_PinMap[PulseNum-1], LOW);
  PulseState=false;
}

void serial_flush(void) {
  NewData=false;
  while (Serial.available()) Serial.read();
}
