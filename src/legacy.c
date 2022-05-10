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



// Bimode
void init_bimode() {
  int historyBits = 1 << bimodeBits;

  choicePHT = (int*) malloc(historyBits * sizeof(int));
  PHTNT = (int*) malloc(historyBits * sizeof(int));
  PHTT = (int*) malloc(historyBits * sizeof(int));

  for(int i = 0; i < historyBits; i++) {
    choicePHT[i] = WN;
    PHTNT[i] = WN;
    PHTT[i] = WN;
  }
  gHistoryTable = 0;
}

uint8_t T_predict(uint32_t pc) {
  int historyBits = 1 << bimodeBits;
  int pc_bits = pc & (historyBits - 1);
  int ghistory_lower = gHistoryTable & (historyBits - 1);
  int index = pc_bits ^ ghistory_lower;

  // choose a prediction based on the table
  switch(PHTT[index]) {
    case SN:
      return NOTTAKEN;
    case WN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in BHT!\n");
      return NOTTAKEN;
  }
}

uint8_t NT_predict(uint32_t pc) {
  int historyBits = 1 << bimodeBits;
  int pc_bits = pc & (historyBits - 1);
  int ghistory_lower = gHistoryTable & (historyBits - 1);
  int index = pc_bits ^ ghistory_lower;

  // choose a prediction based on the table
  switch(PHTNT[index]) {
    case SN:
      return NOTTAKEN;
    case WN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in BHT!\n");
      return NOTTAKEN;
  }
}

uint8_t choicepredict(uint32_t pc) {
  int historyBits = 1 << bimodeBits;
  int pc_lower_bits = pc & (historyBits - 1);
  int ghistory_lower = gHistoryTable & (historyBits - 1);

  // choose a prediction based on the table
  switch(choicePHT[ghistory_lower]) {
    case SN:
      return NOTTAKEN;
    case WN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

uint8_t bimode_predict(uint32_t pc) {
  int historyBits = 1 << bimodeBits;
  
  int pc_lower_bits = pc & (historyBits - 1);

  // choose a prediction based on the table
  if(choicePHT[pc_lower_bits] == ST || choicePHT[pc_lower_bits] == WT){
    return T_predict(pc);
  } else if(choicePHT[pc_lower_bits] == SN || choicePHT[pc_lower_bits] == WN){
   return NT_predict(pc);
  } else{
    printf("something is wrong\n");
    return -1;
  }
}

void train_bimode(uint32_t pc, uint8_t outcome) {
  uint32_t historyBits = 1 << bimodeBits;
  uint32_t pc_lower_bits = pc & (historyBits - 1);
  uint32_t ghistory_lower = gHistoryTable & (historyBits - 1);
  uint32_t historyIndex = pc_lower_bits ^ (ghistory_lower);
  
  // only update the chosen PHT
  if(choicePHT[historyIndex] == SN || choicePHT[historyIndex] == WN){
    switch(PHTNT[historyIndex]) {
      case SN:
        PHTNT[historyIndex] = (outcome == TAKEN) ? WN : SN;
        break;
      case WN:
        PHTNT[historyIndex] = (outcome == TAKEN) ? WT : SN;
        break;
      case WT:
        PHTNT[historyIndex] = (outcome == TAKEN) ? ST : WN;
        break;
      case ST:
        PHTNT[historyIndex] = (outcome == TAKEN) ? ST : WT;
        break;
      default:
        printf("Warning: Undefined state of entry in GSHARE BHT!\n");
        break;
    }
  } else if(choicePHT[historyIndex] == ST || choicePHT[historyIndex] == WT){
    switch(PHTT[historyIndex]) {
      case SN:
        PHTT[historyIndex] = (outcome == TAKEN) ? WN : SN;
        break;
      case WN:
        PHTT[historyIndex] = (outcome == TAKEN) ? WT : SN;
        break;
      case WT:
        PHTT[historyIndex] = (outcome == TAKEN) ? ST : WN;
        break;
      case ST:
        PHTT[historyIndex] = (outcome == TAKEN) ? ST : WT;
        break;
      default:
        printf("Warning: Undefined state of entry in GSHARE BHT!\n");
        break;
    }
  } else{
    printf("something is wrong ;-; \n");
  }

  // update choice PHT if it agrees with branch 
  if(bimode_predict(pc) != outcome && choicepredict(pc) == outcome){
    switch(choicePHT[pc_lower_bits]) {
      case SN:
        choicePHT[pc_lower_bits] = (outcome == TAKEN) ? WN : SN;
        break;
      case WN:
        choicePHT[pc_lower_bits] = (outcome == TAKEN) ? WT : SN;
        break;
      case WT:
        choicePHT[pc_lower_bits] = (outcome == TAKEN) ? ST : WN;
        break;
      case ST:
        choicePHT[pc_lower_bits] = (outcome == TAKEN) ? ST : WT;
        break;
      default:
        printf("Warning: Undefined state of entry in GSHARE BHT!\n");
        break;
      
    }
  }
  
  gHistoryTable = ((gHistoryTable << 1 ) | outcome);
}

void cleanup_bimode() {
  free(choicePHT);
  free(PHTNT);
  free(PHTT);
}