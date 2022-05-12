//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Isamu Poy";
const char *studentID   = "A59011424";
const char *email       = "ipoy@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

//define number of bits required for indexing the BHT here. 
int ghistoryBits = 14; // PREV 13!!!! // Number of bits used for Global History
int bpType;       // Branch Prediction Type
int verbose;

// NEW
#define N 64 // number of weights for perceptron (0th input is 1)
// int history_limit = 10;
int history_mask = 11; // 11
int localhistoryBits = 10; // 10 for alpha spec 
int globalhistoryBits = 12; // 12 for alpha spec (works with tournament now)
int globalhistoryBits2 = 12; // for 2nd attempt for tournament
// int bimodeBits = 13;
//int cacheSize = 8;

int num_entries = 4;
//const int N = 31; 
double num = 1.93;
double theta; // for perceptron training threshold
int y;

// NOTE TO SELF: Gshare and static already given. Implement Tournament and custom (tage)

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
//gshare
int *gpredictors;
int gHistoryTable;

// tournament
int *localpredictors; // local predictors
int *globalpredictors;
// int *globalpredictors2;
int *cpredictors; // choice predictors
int *localHistoryTable; // local history table
int **perceptronTable; // perceptron table
int pghr[N];
int *plhr; // NEW
int num_pghr = 0;
int num_plhr = 0;
int prediction; // for perceptron
int ghr;

// custom (bimode)
int *choicePHT;
int *PHTNT;
int *PHTT;
int *NT_tags;
int *T_tags;
// TODO

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor

//gshare functions
void init_gshare() {
  int historyBits = 1 << ghistoryBits;
  gpredictors = (int*) malloc(historyBits * sizeof(int));

  for(int i = 0; i < historyBits; i++) {
    gpredictors[i] = WN;
  }
  gHistoryTable = 0;
}

uint8_t 
gshare_predict(uint32_t pc) {
  int historyBits = 1 << ghistoryBits;
  int pc_lower_bits = pc & (historyBits - 1);
  int ghistory_lower = gHistoryTable & (historyBits - 1);
  int historyIndex = pc_lower_bits ^ (ghistory_lower);

  switch(gpredictors[historyIndex]) {
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

void
train_gshare(uint32_t pc, uint8_t outcome) {
  uint32_t historyBits = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (historyBits - 1);
  uint32_t ghistory_lower = gHistoryTable & (historyBits - 1);
  uint32_t historyIndex = pc_lower_bits ^ (ghistory_lower);
  
  switch(gpredictors[historyIndex]) {
    case SN:
      gpredictors[historyIndex] = (outcome == TAKEN) ? WN : SN;
      break;
    case WN:
      gpredictors[historyIndex] = (outcome == TAKEN) ? WT : SN;
      break;
    case WT:
      gpredictors[historyIndex] = (outcome == TAKEN) ? ST : WN;
      break;
    case ST:
      gpredictors[historyIndex] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      break;
  }
  gHistoryTable = ((gHistoryTable << 1 ) | outcome);
}

void
cleanup_gshare() {
  free(gpredictors);
}

// ================================== NEW ==================================
// global functions
void init_global() {
  int historyBits = 1 << globalhistoryBits;
  globalpredictors = (int*) malloc(historyBits * sizeof(int));
  for(int i = 0; i < historyBits; i++) {
    globalpredictors[i] = WN;
  }
  ghr = 0;
}

uint8_t global_predict(){;
  uint32_t historyBits = 1 << globalhistoryBits;
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

void train_global(uint8_t outcome) {
  uint32_t historyBits = 1 << globalhistoryBits;
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
cleanup_global() {
  free(globalpredictors);
}

// local functions
void init_local() {
  int historyBits = 1 << localhistoryBits;
  int hist_num = 1 << history_mask;
  //uint32_t history_cutoff = 1 << history_limit;
  localpredictors = (int*) malloc(hist_num * sizeof(int));
  localHistoryTable = (int*) malloc(historyBits * sizeof(int));
  
  // TODO: build a hash table for the local history table indexed up to 2^10 possible addresses 0 -> 1111111111
  // first empty, if the address is not found, and the map is not full, add the key and value.
  // note localpredictors also need to be maps of local histories...

  for(int i = 0; i < historyBits; i++) {
    localHistoryTable[i] = 0; // 1023 of them, but the indices won't be the same! need a map
  }

  for(int i = 0; i < hist_num; i++) {
    localpredictors[i] = WN;
  }

  //printf("\nlocal init done\n");
}

uint8_t local_predict(uint32_t pc) {
  int historyBits = 1 << localhistoryBits;
  int pc_bits = pc & (historyBits - 1);
  
  int hist_num = 1 << history_mask;
  //printf("\nstarting predict\n");
  int index = localHistoryTable[pc_bits] & (hist_num - 1);

  switch(localpredictors[index]) {
    case SN:
      return NOTTAKEN;
    case WN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Undefined state in predictor table");
      return NOTTAKEN;
  }
}

void train_local(uint32_t pc, uint8_t outcome) {
  uint32_t historyBits = 1 << localhistoryBits;
  uint32_t pc_bits = pc & (historyBits - 1);
  //uint32_t ghistory_lower = gHistoryTable & (historyBits - 1);
  //uint32_t historyIndex = pc_lower_bits ^ (ghistory_lower);
  
  int hist_num = 1 << history_mask;
  //printf("\nstarting predict\n");
  int index = localHistoryTable[pc_bits] & (hist_num - 1);

  switch(localpredictors[index]) {
    case SN:
      localpredictors[index] = (outcome == TAKEN) ? WN : SN;
      break;
    case WN:
      localpredictors[index] = (outcome == TAKEN) ? WT : SN;
      break;
    case WT:
      localpredictors[index] = (outcome == TAKEN) ? ST : WN;
      break;
    case ST:
      localpredictors[index] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      break;
  }
  localHistoryTable[pc_bits] = ((localHistoryTable[pc_bits] << 1 ) | outcome);
  //printf("training done\n");
}

void cleanup_local() {
  free(localpredictors);
}

// tournament functions
void init_tour(){

  // init global prediction table (12 bits)
  init_global();
  // init local history table (10 bits)
  init_local();
  
  // initialize the choice prediction table (size 12 bits)
  int historyBits = 1 << globalhistoryBits;
  cpredictors = (int*) malloc(historyBits * sizeof(int));

  for(int i = 0; i < historyBits; i++) { // set WN to 2^10 states
    cpredictors[i] = WN;
  }
}

uint8_t tour_predict(uint32_t pc) { 
  // conduct global history prediction
  // choice BHT prediction final mux
  int historyBits = 1 << globalhistoryBits;
  int ghr_lower = ghr & (historyBits - 1);

  switch(cpredictors[ghr_lower]) {
    case SN:
      return local_predict(pc);
    case WN:
      return local_predict(pc);
    case WT:
      return global_predict();
    case ST:
      return global_predict();
    default:
      printf("Undefined state in predictor table 2 ");
      return NOTTAKEN;
  }

  printf("\ntour predict done\n");
}

// QUESTION: how does the ghr get limited to 12 bits...
void train_tour(uint32_t pc, uint8_t outcome) {
  // train choice 
  uint32_t historyBits = 1 << globalhistoryBits;
  uint32_t ghr_lower = ghr & (historyBits - 1);
  
  if(local_predict(pc) != global_predict()){
    if(outcome == local_predict(pc)){
      switch(cpredictors[ghr_lower]) {
        case SN:
          cpredictors[ghr_lower] = SN;
          break;
        case WN:
          cpredictors[ghr_lower] = SN;
          break;
        case WT:
          cpredictors[ghr_lower] = WN;
          break;
        case ST:
          cpredictors[ghr_lower] = WT;
          break;
        default:
          break;
      }
    } else if(outcome == global_predict()){
      switch(cpredictors[ghr_lower]) {
        case SN:
          cpredictors[ghr_lower] = WN;
          break;
        case WN:
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
  // train local
  train_local(pc, outcome); 
  // train global
  train_global(outcome);

  //printf("choice training done\n");
  ghr = ((ghr << 1 ) | outcome);

}

void cleanup_tour() {
  free(globalpredictors);
  free(cpredictors);
  free(localpredictors);
  free(gpredictors);
}

// custom
// perceptron
void init_percep(){
  int weightBits = 1 << num_entries; // number of columns
  theta = 1.93*N + 14;
  perceptronTable = (int**) malloc((weightBits) * sizeof(int*));

  // allocate array of weights for each entry of the perceptron table (indexed by addresses)
  for(int i = 0; i < weightBits; i++) { // set WN to 2^10 states
    perceptronTable[i] = (int*) malloc((N+1) * sizeof(int));

    for(int j = 0; j <= N; j++){
      perceptronTable[i][j] = 0;
    }
  }

  for(int i = 0; i < N; i++){
    pghr[i] = 0;
  }

  ghr = 0;
}

uint8_t percep_predict(uint32_t pc) { 
  uint32_t historyBits = 1 << num_entries;
  uint32_t pc_bits = pc & (historyBits - 1);
  int ghistory_lower = ghr & (historyBits - 1);
  pc_bits = pc_bits ^ (ghistory_lower);

  y = 0;

  for(int i = 0; i <= N; i++){
    if(i == 0){
      y += perceptronTable[pc_bits][0];
    } else{
      y += pghr[i-1]*perceptronTable[pc_bits][i];
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
  uint32_t historyBits = 1 << num_entries;
  uint32_t pc_bits = pc & (historyBits - 1);
  int ghistory_lower = ghr & (historyBits - 1);
  pc_bits = pc_bits ^ (ghistory_lower);

  y = 0;

  for(int i = 0; i <= N; i++){
    if(i == 0){
      y += perceptronTable[pc_bits][0];
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
  if((prediction != outcome) || abs(y) <= theta){
    for(int i = 0; i <= N; i++){
      if(outcome == NOTTAKEN){
        if(i == 0){
          perceptronTable[pc_bits][0] += -1;
        } else{
          perceptronTable[pc_bits][i] += -1*pghr[i-1];
        }
      } else if(outcome == TAKEN){
        if(i == 0){
          perceptronTable[pc_bits][0] += 1;
        } else{
          perceptronTable[pc_bits][i] += 1*pghr[i-1];
        }
      } else{
        printf("nooo\n");
      }
    }
  }

  if(num_pghr < N){
    if(outcome == TAKEN) {
      pghr[num_pghr] = 1;
    } else {
      pghr[num_pghr] = -1;
    }
    num_pghr++; 
  } else{
    for(int i = N-1; i >= 1; i--){
      pghr[i] = pghr[i-1];
    }

    if(outcome == TAKEN) {
      pghr[0] = 1;
    } else {
      pghr[0] = -1;
    }
  }
  ghr = ((ghr << 1 ) | outcome);
}

void cleanup_percep() {
  free(perceptronTable);
}

void init_gshare2() {
  int size = 13;
  int historyBits = 1 << size;
  gpredictors = (int*) malloc(historyBits * sizeof(int));

  for(int i = 0; i < historyBits; i++) {
    gpredictors[i] = WN;
  }

  ghistoryBits = 0;
}

uint8_t gshare2_predict(uint32_t pc) {
  int size = 13;
  int historyBits = 1 << size;
  int pc_lower_bits = pc & (historyBits - 1);
  int ghistory_lower = ghistoryBits & (historyBits - 1);
  int historyIndex = pc_lower_bits ^ (ghistory_lower);

  switch(gpredictors[historyIndex]) {
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

void train_gshare2(uint32_t pc, uint8_t outcome) {
  int size = 13;
  int historyBits = 1 << size;
  int pc_lower_bits = pc & (historyBits - 1);
  int ghistory_lower = ghistoryBits & (historyBits - 1);
  int historyIndex = pc_lower_bits ^ (ghistory_lower);
  
  switch(gpredictors[historyIndex]) {
    case SN:
      gpredictors[historyIndex] = (outcome == TAKEN) ? WN : SN;
      break;
    case WN:
      gpredictors[historyIndex] = (outcome == TAKEN) ? WT : SN;
      break;
    case WT:
      gpredictors[historyIndex] = (outcome == TAKEN) ? ST : WN;
      break;
    case ST:
      gpredictors[historyIndex] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      break;
  }
}

void
cleanup_gshare2() {
  free(gpredictors);
}

void init_percepTour(){

  // init global prediction table (12 bits)
  init_percep();
  // init local history table (10 bits)
  init_gshare2();
  
  // initialize the choice prediction table (size 12 bits)
  int historyBits = 1 << num_entries;
  cpredictors = (int*) malloc(historyBits * sizeof(int));

  for(int i = 0; i < historyBits; i++) { // set WN to 2^10 states
    cpredictors[i] = WN;
  }

  ghr = 0;
}

uint8_t percepTour_predict(uint32_t pc) { 
  // conduct global history prediction
  // choice BHT prediction final mux
  int historyBits = 1 << num_entries;
  int ghr_lower = ghr & (historyBits - 1);

  switch(cpredictors[ghr_lower]) {
    case SN:
      return gshare2_predict(pc);
    case WN:
      return gshare2_predict(pc);
    case WT:
      return percep_predict(pc);
    case ST:
      return percep_predict(pc);
    default:
      printf("Undefined state in predictor perceptrons ");
      return NOTTAKEN;
  }
}

void train_percepTour(uint32_t pc, uint8_t outcome) {
  // train choice 
  uint32_t historyBits = 1 << num_entries;
  uint32_t ghr_lower = ghr & (historyBits - 1);
  
  if(percep_predict(pc) != gshare2_predict(pc)){
    if(outcome == gshare2_predict(pc)){
      switch(cpredictors[ghr_lower]) {
        case SN:
          cpredictors[ghr_lower] = SN;
          break;
        case WN:
          cpredictors[ghr_lower] = SN;
          break;
        case WT:
          cpredictors[ghr_lower] = WN;
          break;
        case ST:
          cpredictors[ghr_lower] = WT;
          break;
        default:
          break;
      }
    } else if(outcome == percep_predict(pc)){
      switch(cpredictors[ghr_lower]) {
        case SN:
          cpredictors[ghr_lower] = WN;
          break;
        case WN:
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
  // train local
  train_percep(pc, outcome); 
  // train global
  train_gshare2(pc, outcome);

  //printf("choice training done\n");
  ghr = ((ghr << 1 ) | outcome);

}


// ============================================= //
//                 non-dev code                  //
// ============================================= //

void
init_predictor()
{
  switch (bpType) {
    case STATIC:
    case GSHARE:
      init_gshare();
      break;
    case TOURNAMENT:
      init_tour();
      break;
    case CUSTOM:
      init_percepTour();
      break;
    default:
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN; 
    case GSHARE:
      return gshare_predict(pc);
    case TOURNAMENT:
      return tour_predict(pc);
    case CUSTOM:
      return percepTour_predict(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)

void
train_predictor(uint32_t pc, uint8_t outcome)
{

  switch (bpType) {
    case STATIC:
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return train_tour(pc, outcome);
    case CUSTOM:
      return train_percepTour(pc, outcome);
    default:
      break;
  }
  

}
