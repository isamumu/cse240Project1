// custom predictor 
void init_percep(){
  int weightBits = 1 << N;
  perceptronTable = (int**) malloc((weightBits) * sizeof(int*));

  // allocate array of weights for each entry of the perceptron table (indexed by addresses)
  for(int i = 0; i < weightBits; i++) { // set WN to 2^10 states
    perceptronTable[i] = (int*) malloc(N * sizeof(int));

    for(int j = 0; j < N; j++){
      perceptronTable[i][j] = 0;
    }
  }

  for(int i = 0; i < N-1; i++){
    pghr[i] = 0;
  }
}

uint8_t percep_predict(uint32_t pc) { 
  uint32_t historyBits = 1 << N;
  uint32_t pc_bits = pc & (historyBits - 1);

  y = 0;

  for(int i = 0; i < N; i++){
    if(i == 0){
      y += 1*perceptronTable[pc_bits][i];
    } else{
      y += pghr[i]*perceptronTable[pc_bits][i];
    }
  }

  // comparator
  if(y > 0){
    return TAKEN;
  } else{
    return NOTTAKEN;
  } 
}

void train_percep(uint32_t pc, uint8_t outcome) {
  uint32_t historyBits = 1 << N;
  uint32_t pc_bits = pc & (historyBits - 1);

  y = 0;

  for(int i = 0; i < N; i++){
    if(i == 0){
      y += 1*perceptronTable[pc_bits][i];
    } else{
      y += pghr[i-1]*perceptronTable[pc_bits][i];
    }
  }

  // comparator
  if(y > 0){
    prediction = TAKEN;
  } else{
    prediction = NOTTAKEN;
  } 

  // update the weights in perceptron table
  if((prediction != outcome) || abs(y) < theta){
    for(int i = 0; i < N; i++){
      if(i == 0){
        perceptronTable[pc_bits][i] = outcome;
      } else{
        perceptronTable[pc_bits][i] += outcome*pghr[i-1];
      }
    }
  }

  if(num_pghr < N){
    pghr[num_pghr] = outcome;
    num_pghr++; 
  } else{
    for(int i = N-1; i >= 1; i--){
      pghr[i] = pghr[i-1];
    }
    pghr[0] = outcome;
  }

}

void cleanup_percep() {
  free(perceptronTable);
}

// custom attempt 2
// tournament functions
