
// custom attempt 2
// tournament functions

void init_global2() {
  int historyBits = 1 << globalhistoryBits2;
  globalpredictors = (int*) malloc(historyBits * sizeof(int));
  for(int i = 0; i < historyBits; i++) {
    globalpredictors[i] = WT;
  }
  ghr = 0;
}

uint8_t global2_predict(){;
  uint32_t historyBits = 1 << globalhistoryBits2;
  uint32_t ghr_lower = ghr & (historyBits - 1);

  //printf("\nprediction start\n");
  switch(globalpredictors[ghr_lower]) {
    case SN:
      return NOTTAKEN;
    case WN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      return NOTTAKEN;
  }
}

void train_global2(uint8_t outcome) {
  uint32_t historyBits = 1 << globalhistoryBits2;
  uint32_t ghr_lower = ghr & (historyBits - 1);

  switch(globalpredictors[ghr_lower]) {
    case SN:
      globalpredictors[ghr_lower] = (outcome == TAKEN) ? WN : SN;
      break;
    case WN:
      globalpredictors[ghr_lower] = (outcome == TAKEN) ? WT : SN;
      break;
    case WT:
      globalpredictors[ghr_lower] = (outcome == TAKEN) ? ST : WN;
      break;
    case ST:
      globalpredictors[ghr_lower] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      break;
  }
  //ghr = ((ghr << 1 ) | outcome);
}

void
cleanup_global2() {
  free(globalpredictors);
}



void init_tour2(){
  // init global prediction table (12 bits)
  init_global2();
  init_gshare2();
  //init_local2();
  
  // initialize the choice prediction table (size 12 bits)
  int historyBits = 1 << globalhistoryBits2;
  cpredictors = (int*) malloc(historyBits * sizeof(int));

  for(int i = 0; i < historyBits; i++) { // set WN to 2^10 states
    cpredictors[i] = WN; // init to WWN instead of WN
  }
  printf("init done\n");
  ghr = 0;
}

// local vs. gshare tournament with hysterisis
uint8_t tour2_predict(uint32_t pc) { 
  // conduct global history prediction
  // choice BHT prediction final mux
  int historyBits = 1 << globalhistoryBits;
  int ghr_lower = ghr & (historyBits - 1);

  switch(cpredictors[ghr_lower]) {
    case SN:
      return gshare2_predict(pc);
    case WN:
      return gshare2_predict(pc);
    case WWN:
      return gshare2_predict(pc);
    case WWT:
      return global2_predict();
    case WT:
      return global2_predict();
    case ST:
      return global2_predict();
    default:
      printf("Undefined state in predictor table 2 ");
      return NOTTAKEN;
  }
}

// QUESTION: how does the ghr get limited to 12 bits...
void train_tour2(uint32_t pc, uint8_t outcome) {
  // train choice 
  uint32_t historyBits = 1 << globalhistoryBits;
  uint32_t ghr_lower = ghr & (historyBits - 1);
  
  if(gshare2_predict(pc) != global2_predict()){
    if(outcome == gshare2_predict(pc)){
      switch(cpredictors[ghr_lower]) {
        case SN:
          cpredictors[ghr_lower] = SN;
        break;
        case WN:
          cpredictors[ghr_lower] = SN;
          break;
        case WWN:
          cpredictors[ghr_lower] = WN;
          break;
        case WWT:
          cpredictors[ghr_lower] = WWN;
          break;
        case WT:
          cpredictors[ghr_lower] = WWT;
          break;
        case ST:
          cpredictors[ghr_lower] = WT;
          break;
        default:
          break;
      }
    } else if(outcome == global2_predict()){
      switch(cpredictors[ghr_lower]) {
        case SN:
          cpredictors[ghr_lower] = WN;
        break;
        case WN:
          cpredictors[ghr_lower] = WWN;
          break;
        case WWN:
          cpredictors[ghr_lower] = WWT;
          break;
        case WWT:
          cpredictors[ghr_lower] = WT;
          break;
        case WT:
          cpredictors[ghr_lower] = ST;
          break;
        case ST:
          cpredictors[ghr_lower] = ST;
          break;
        default:
          break;
      }
    } else{
      printf("bug detected\n");
    }
  }
  // train 
  train_gshare2(pc, outcome); 
  //train_global2(outcome);
  train_global2(pc);

  ghr = ((ghr << 1 ) | outcome);

}

void cleanup_tour2() {
  free(globalpredictors);
  free(cpredictors);
  free(gpredictors);
  free(localpredictors);
}

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