// Programa para demostração da depuração no ESP32-S3

#define TAM_TAB 100
int tabela[TAM_TAB];

void setup() {
  Serial.begin(115200);
  Serial.println("Demo depuração");
  preenche_tabela();
  ordena_tabela();
  Serial.print("Minimo: ");
  Serial.println(tabela[0]);
}

void loop() {
  Serial.println("rodando...");
  delay(60000);
}

void preenche_tabela() {
  for (int i = 0; i < TAM_TAB; i++) {
    tabela[i] = (int) random(1000);
  }
}

void ordena_tabela() {
  for (int i = 1; i < TAM_TAB; i++) {
    for (int j = i; (j > 0) && (tabela[j] < tabela[j-1]); j--) {
      int aux = tabela[j];
      tabela[j] = tabela[j-1];
      tabela[j-1] = aux;
    }
  }
}