#include <RF24Network.h> 
#include <RF24.h> 
#include <SPI.h>
#include <EEPROM.h>


RF24 radio(9,10); //Cria o objeto Radio para se comunicar e controlar o nRF24l01
RF24Network network(radio); // Cria o objeto da rede para usar o mesh

int endNo; //endereço do nó
const uint16_t rXNode= 00; // endereço do cordenador , sempre 00
bool timer = true; // Usado enquanto modo roreador para determinar o tempo do intervalo  
byte Modo;        //variaveis para envio na rede
byte tipo;
byte sIntervalo;
int portaLDR = A5; //Porta analógica utilizada pelo LDR
int leitura;         //Armazena informações sobre a leitura do sensor Hall
unsigned long ultimoE;
unsigned long intervaloT;
unsigned long intervaloI;


//Estrutura usada para criar e determinar as informçaões a ser transmitidas
//pelo pacote.Lembrando que quanto mais informações mais chance de erro na recepção.
//kept simple.
struct payload_t 
{
  int nNo; //<- armazena o numero do nó
  byte Interval; //<- armazena o info1o de trasnmissão 
  byte mmodo;//<- armazena o modo de trabalho do nó
  byte Tipo;// <- Armazena o tipo do sensor
  float info1; //<- armazena  a informação 1 do nó
  float info2; // <- armazena a informação 2 do nó
  };
  
void setup() {
  tipo = 01;
  pinMode(8, INPUT_PULLUP); 
  pinMode(7,OUTPUT);
  ConfigurarNo();
  SPI.begin();//Inicia comunicação SPI
  radio.begin(); //inicia a comunicação de radio com o modulo
  network.begin(90,endNo);
  
}

void loop() {
  
  //Verifica a rede regularmente
   network.update();
   timer = transmitir();   
      //esta na hora de transmitir?
  if(timer)
  {           
    payload_t payload ={endNo,sIntervalo,Modo,tipo,LerLDR()}; //faz o pacote a ser transmitido
    RF24NetworkHeader header(rXNode); // monta o cabeçalho de rede
    if(network.write(header,&payload,sizeof(payload)))
    {
      digitalWrite(7,LOW);
     }
    else{
      PIND |= (1<<PIND7);
         }       
    }
     
}

int LerLDR()//faz leitura do sensor Hall
{
  leitura = analogRead(portaLDR);
  return leitura ;   
}

bool transmitir() //<----- Verifica se é tempo de transmitir
{
  intervaloI = millis();
  if( intervaloI - ultimoE >= intervaloT)
  {
    ultimoE = intervaloI;
    return true;
    }else
    {
      return false;
    }  
}

//Função de configuração do nó de rede , nele será configurado o endereço e e o intervalo
//e o intervalo de transmissão , como o tipo do nó.
void ConfigurarNo()
{
  pinMode(8, INPUT_PULLUP);  
  int valor;//variavel apra armazenar o endereço do nó(octal)
  byte cEnd = 128; //verifica se é a primeira vez que é executada a configuração 
  byte tInterval;// determina o tempo de transmissão
  byte cmodo; // seta se eh router ou no 
  String entra = F("Entre 'S' para sim ou qualquer outro caracter para nao: "); //Macro F() diz para o compilador guardar a string na memoria flash e nao na Sram 
  String invalido = F("Entrada invalida,valor definido para: ");
  String gostaria = F("Gostaria de atualizar o ");

  //Testa se é a primeira vez que a configuração esta sendo executada  Se sim realiza a configuração 
  //capturando as informações  que estão no eeprom e mostra no monitor serial.
  //Importante ressaltar que  para entra no modo configuração o botão tem que estar apertado ao ligar 
  //o no´(router)

   Serial.begin(57600); //inicilaiza o monitor serial.     
  if(EEPROM.get(0,cEnd) != 128 || !digitalRead(8) )
  {
    digitalWrite(7,HIGH);
    //Serial.begin(57600);
    Serial.println(F("Configuracoes gravadas no EEPROM"));
    Serial.print("Endereco do no  :  ");
    Serial.println(EEPROM.get(1,valor),OCT);
    Serial.print("Intervalo de Transmissao : ");
    Serial.println(pegaIntervalo());
    Serial.print("Modo: ");
    Serial.println(pegaModo());
    //Dá as opções de configurações ao usuario
    Serial.print(gostaria);
    Serial.print("endereco do No? ");
    Serial.println(entra);
    if(pegaResposta())
    {
      Serial.println(F("entre o endereco para ser armazenado na EEPROM"));
      while(!Serial.available()){}
      valor = Serial.parseInt();
      if(valor >=0){EEPROM.put(1,valor);}
      else{
        Serial.print(invalido);
        Serial.println("01");
        valor = 01;
        EEPROM.put(1,valor);
      }      
     }
    Serial.print(gostaria);
    Serial.print("intervalo de trasmissao ");
    Serial.println(entra);
    if(pegaResposta())
    {
      Serial.println(F("Entre: 0 para 2 seg, 1 para 1 min , 2 para 10 min e 3 para 15 min"));
      while(!Serial.available()){}
      sIntervalo = Serial.parseInt();
      if(sIntervalo >=0 || sIntervalo <4)
      {
       EEPROM.put(7,sIntervalo); 
      }
      else
      {
        Serial.print(invalido);
        Serial.println("3");
        sIntervalo = 3;
        EEPROM.put(7,sIntervalo);
       }
       }
      Serial.print(gostaria);
      Serial.println(F(" o modo? "));
      Serial.println(entra);
      if(pegaResposta()){
        Serial.println(F("Entre O para modo No ou 1 para modo Router"));
        while(!Serial.available()){}
        cmodo = Serial.parseInt();
          if(cmodo ==0 || cmodo == 1)
            {
              EEPROM.put(3,cmodo);
           }
           else{
                Serial.print(invalido);
                Serial.println("0");
                cmodo =0;
                EEPROM.put(3,cmodo);
           }
      }
  }
  CapturaValoresEEPROM();//caso a  encontre o  valor 128 no campo 0 da EEPROM realiza a busca e grava
  //nas variaveis globais
  Serial.print("Endereco do no:");
  Serial.println(endNo,OCT);
  Serial.print("Modo do no:");
  Serial.println(pegaModo());
  Serial.print("Intervalo de tempo de transmissao:");
  Serial.println(pegaIntervalo());
  cEnd = 128; // Coloca o valor "128" no EEPROM para mostrar que a configuração foi feita ao menos uma vez
  EEPROM.put(0,cEnd);
  digitalWrite(7,LOW); //desliga o led de configuração
  Serial.end();//finaliza a comunicação serial, talvez reative em testes 
    
}
void CapturaValoresEEPROM() {
  EEPROM.get(1,endNo);
  EEPROM.get(3,Modo);
  EEPROM.get(7,sIntervalo);
 }
String pegaIntervalo() {
  byte i;
  String m = " min";
  EEPROM.get(7,i);
  if(i==0) {
    intervaloT = 2000;
    return "2 seg";
  }
  else if (i==1) {
    intervaloT = 60000;
    return ("1"+m);
  }
  else if (i==2) {
    intervaloT = 600000;
    return ("10"+m);
  }
  else {
     EEPROM.put(7,i);
     intervaloT = 900000;
    return ("15"+m);
  }
}
String pegaModo() {
  byte m;
  EEPROM.get(3,m);
  if(m==1) {
    return "Router";
  }
  else {
    m = 0;
    EEPROM.put(3,m);
    return "No";
  }
}
bool pegaResposta() {
   while (!Serial.available()) { }
   if(Serial.read() == 'S' || Serial.read() == 's') return true;
   else return false;
}
