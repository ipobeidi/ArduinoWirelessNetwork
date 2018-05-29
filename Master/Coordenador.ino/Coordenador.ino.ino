#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

RF24 radio(9,10);
//Inicia a Network do radio
RF24Network network(radio);

//endereço do coordenador
const uint16_t this_node = 0;

  
//Estrutura do pacote de rede
struct payload_t
{
  int nNo; //<- armazena o numero do nó
  byte Interval; //<- armazena o info1o de trasnmissão 
  byte mmodo;//<- armazena o modo de trabalho do nó
  byte Tipo;// <- Armazena o tipo do sensor
  float info1; //<- armazena  a informação 1 do nó
  float info2; // <- armazena a informação 2 do nó
  };

void setup(void)
{
  Serial.begin(57600);
  Serial.println("Cordenador esta online"); 
  SPI.begin();
  radio.begin();
  network.begin( 90,this_node);
}

void loop(void)
{
  // Atualiza a rede regularmente
  network.update();
  // Tem algum pacote?
  while ( network.available() )
  {
    //se sim captura e imprime ele 
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    //printa no monitor serial as informações
      Serial.print("Recebido pacote do No:");
      Serial.print(payload.nNo);      
      Serial.print("Funcionando no modo:  ");
      Serial.println(payload.mmodo);
      Serial.print(" Intervalo de transmissão:");
      Serial.println(payload.Interval);
      
        
    if(payload.Tipo == 01)
    {      
      Serial.print("Temperatura: ");
      Serial.println(payload.info1);
      Serial.print("Umidade: ");
      Serial.println(payload.info2);
     }
    else
      if(payload.Tipo==02){
               Serial.print("Distancia: ");
               Serial.println(payload.info1);
      }
      else
          if(payload.Tipo==0){
            Serial.print("Informação");
            Serial.println(payload.info1);
      }
   
    }
  }

