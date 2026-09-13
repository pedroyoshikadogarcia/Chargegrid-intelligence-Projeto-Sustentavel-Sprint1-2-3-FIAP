const int POT_CARRO1 = A0;
const int POT_CARRO2 = A1;
const int BOTAO_PICO = 2;

const int LED_AZUL = 10;
const int LED_VERDE = 11;
const int LED_AMARELO = 12;
const int LED_VERMELHO = 13;

const float LIMITE_CONTRATO = 45.0;
const float BATERIA_MAX = 15.0;

bool horarioPico = false;
int estadoBotaoAnt = LOW;

unsigned long tempoAnterior = 0;
const long intervaloExibicao = 3000;

void setup() {
  Serial.begin(9600);
  pinMode(BOTAO_PICO, INPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  
  Serial.println("--- CHARGEGRID INTELLIGENCE v3.0 (INTEGRATED ESS) ---");
}

void loop() {
  int estadoBotao = digitalRead(BOTAO_PICO);
  if (estadoBotao == HIGH && estadoBotaoAnt == LOW) {
    horarioPico = !horarioPico;
    delay(50);
  }
  estadoBotaoAnt = estadoBotao;

  float demandaCarro1 = map(analogRead(POT_CARRO1), 0, 1023, 0, 30);
  float demandaCarro2 = map(analogRead(POT_CARRO2), 0, 1023, 0, 30);
  float totalSolicitado = demandaCarro1 + demandaCarro2;

  float cargaRealCarro1 = demandaCarro1;
  float cargaRealCarro2 = demandaCarro2;
  float totalEntregue = totalSolicitado;
  bool controleAtivoAcionado = false;

  if (totalSolicitado > LIMITE_CONTRATO) {
    controleAtivoAcionado = true;
    float fatorCorte = LIMITE_CONTRATO / totalSolicitado;
    cargaRealCarro1 = demandaCarro1 * fatorCorte;
    cargaRealCarro2 = demandaCarro2 * fatorCorte;
    totalEntregue = LIMITE_CONTRATO;
  }

  float injecaoBateria = 0.0;
  bool bateriaAtiva = false;

  if (horarioPico && totalEntregue > 0) {
    bateriaAtiva = true;
    injecaoBateria = (totalEntregue < BATERIA_MAX) ? totalEntregue : BATERIA_MAX;
  }

  float cargaRede = totalEntregue - injecaoBateria;

  float precoKwh = horarioPico ? 2.40 : 0.90;

  digitalWrite(LED_AZUL, bateriaAtiva ? HIGH : LOW);
  digitalWrite(LED_AMARELO, horarioPico ? HIGH : LOW);
  digitalWrite(LED_VERMELHO, controleAtivoAcionado ? HIGH : LOW);
  digitalWrite(LED_VERDE, (!horarioPico && !controleAtivoAcionado) ? HIGH : LOW);

  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervaloExibicao) {
    tempoAnterior = tempoAtual;

    Serial.println("\n=====================================");
    Serial.print("STATUS DA REDE: ");
    if (horarioPico) Serial.println("HORARIO DE PICO (Tarifa Elevada)");
    else Serial.println("HORARIO NORMAL (Tarifa Reduzida)");
    
    Serial.print("Preco kWh: R$ "); Serial.println(precoKwh);
    Serial.print("Injecao Bateria GoodWe Lynx: "); Serial.print(injecaoBateria); Serial.println(" kW (Peak Shaving)");
    Serial.print("Demanda Exigida da Concessionaria: "); Serial.print(cargaRede); Serial.print(" / "); Serial.print(LIMITE_CONTRATO); Serial.println(" kW");
    
    Serial.print("Carro 1 -> Pedido: "); Serial.print(demandaCarro1); Serial.print("kW | Entregue: "); Serial.print(cargaRealCarro1); Serial.println("kW");
    Serial.print("Carro 2 -> Pedido: "); Serial.print(demandaCarro2); Serial.print("kW | Entregue: "); Serial.print(cargaRealCarro2); Serial.println("kW");
    
    if (controleAtivoAcionado) {
      Serial.println("ALERTA: CONTROLE ATIVO + BATERIA NO LIMITE! CORTE APLICADO.");
    }
  }
}
