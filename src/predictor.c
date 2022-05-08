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
int ghistoryBits = 13; // PREV 13!!!! // Number of bits used for Global History
int bpType;       // Branch Prediction Type
int verbose;

// NEW
int localhistoryBits = 10; 
int globalhistoryBits = 12; // 12 origin
int globalhistoryBits2 = 28; // for 2nd attempt for tournament
const int N = 16; // number of weights for perceptron (0th input is 1)
double theta = 1.93*N + 14; // for perceptron training threshold
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
int *globalpredictors2;
int *cpredictors; // choice predictors
int *localHistoryTable; // local history table
int **perceptronTable; // perceptron table
int pghr[N-1];
int num_pghr = 0;
int prediction; // for perceptron
int ghr;

// custom (tage)
// TODO

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor

//gshare functions
void init_gshare() {
  int historyBits = 1 << ghistoryBits;
  gpredictors = (int*) malloc(historyBits * sizeof(int));

  for(int i = 0; i <= historyBits; i++) {
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
      printf("Undefined state in predictor table");
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
  localpredictors = (int*) malloc(historyBits * sizeof(int));
  localHistoryTable = (int*) malloc(historyBits * sizeof(int));
  
  // TODO: build a hash table for the local history table indexed up to 2^10 possible addresses 0 -> 1111111111
  // first empty, if the address is not found, and the map is not full, add the key and value.
  // note localpredictors also need to be maps of local histories...

  for(int i = 0; i < historyBits; i++) {
    localpredictors[i] = WN;
    localHistoryTable[i] = 0; // 1023 of them, but the indices won't be the same! need a map
  }
  //printf("\nlocal init done\n");
}

uint8_t local_predict(uint32_t pc) {
  int historyBits = 1 << localhistoryBits;
  int pc_bits = pc & (historyBits - 1);
  // int ghistory_lower = gHistoryTable & (historyBits - 1);
  // int historyIndex = pc_lower_bits ^ (ghistory_lower);
  
  //printf("\nstarting predict\n");
  int index = localHistoryTable[pc_bits] & (historyBits - 1);
  switch(localpredictors[index]) {
    case SN:
      //printf("\nlocal predicted\n");
      return NOTTAKEN;
    case WN:
      //printf("\nlocal predicted\n");
      return NOTTAKEN;
    case WT:
      //printf("\nlocal predicted\n");
      return TAKEN;
    case ST:
      //printf("\nlocal predicted\n");
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
  
  // printf("pc = %u\n", pc_bits);
  // printf("hi: %u\n", localHistoryTable[pc_bits]);
  // printf("\n--->%u\n",localpredictors[localHistoryTable[pc_bits]]);
  int index = localHistoryTable[pc_bits] & (historyBits - 1);
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

  //init_gshare();
  
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
  // train local
  train_local(pc, outcome); 
  // train global
  train_global(outcome);

  // train choice 
  uint32_t historyBits = 1 << globalhistoryBits;
  uint32_t ghr_lower = ghr & (historyBits - 1);
  
  switch(cpredictors[ghr_lower]) {
    case SN:
      cpredictors[ghr_lower] = (outcome == TAKEN) ? WN : SN;
      break;
    case WN:
      cpredictors[ghr_lower] = (outcome == TAKEN) ? WT : SN;
      break;
    case WT:
      cpredictors[ghr_lower] = (outcome == TAKEN) ? ST : WN;
      break;
    case ST:
      cpredictors[ghr_lower] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      break;
  }
  //printf("choice training done\n");
  ghr = ((ghr << 1 ) | outcome);

}

void cleanup_tour() {
  free(globalpredictors);
  free(cpredictors);
  free(localpredictors);
  free(gpredictors);
}

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
void init_global2() {
  int historyBits = 1 << globalhistoryBits2;
  globalpredictors2 = (int*) malloc(historyBits * sizeof(int));
  for(int i = 0; i < historyBits; i++) {
    globalpredictors2[i] = WN;
  }
  ghr = 0;
}

uint8_t global2_predict(uint8_t pc){;
  uint32_t historyBits = 1 << globalhistoryBits2;
  uint32_t ghr_lower = ghr & (historyBits - 1);


  int pc_lower_bits = pc & (historyBits - 1);
  ghr_lower= pc_lower_bits ^ (ghr_lower);

  //printf("\nprediction start\n");
  switch(globalpredictors2[ghr_lower]) {
    case SN:
      return NOTTAKEN;
    case WN:
      return NOTTAKEN;
    case WWN:
      return NOTTAKEN;
    case WWT:
      return TAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      return NOTTAKEN;
  }
}

void train_global2(uint8_t pc, uint8_t outcome) {
  uint32_t historyBits = 1 << globalhistoryBits2;
  uint32_t ghr_lower = ghr & (historyBits - 1);

  int pc_lower_bits = pc & (historyBits - 1);
  ghr_lower= pc_lower_bits ^ (ghr_lower);

  switch(globalpredictors2[ghr_lower]) {
    case SN:
      globalpredictors2[ghr_lower] = (outcome == TAKEN) ? WN : SN;
      break;
    case WN:
      globalpredictors2[ghr_lower] = (outcome == TAKEN) ? WWN : SN;
      break;
    case WWN:
      globalpredictors2[ghr_lower] = (outcome == TAKEN) ? WWT : WN;
      break;
    case WWT:
      globalpredictors2[ghr_lower] = (outcome == TAKEN) ? WT : WWN;
      break;
    case WT:
      globalpredictors2[ghr_lower] = (outcome == TAKEN) ? ST : WWT;
      break;
    case ST:
      globalpredictors2[ghr_lower] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      break;
  }
  //ghr = ((ghr << 1 ) | outcome);
}
void init_tour2(){

  // init global prediction table (12 bits)
  init_global2();
  init_gshare();
  
  // initialize the choice prediction table (size 12 bits)
  int historyBits = 1 << globalhistoryBits2;
  cpredictors = (int*) malloc(historyBits * sizeof(int));

  for(int i = 0; i < historyBits; i++) { // set WN to 2^10 states
    cpredictors[i] = WN;
  }
}

uint8_t tour2_predict(uint32_t pc) { 
  // conduct global history prediction
  // choice BHT prediction final mux
  int historyBits = 1 << globalhistoryBits2;
  int ghr_lower = ghr & (historyBits - 1);

  switch(cpredictors[ghr_lower]) {
    case SN:
      return gshare_predict(pc);
    case WN:
      return gshare_predict(pc);
    case WT:
      return global2_predict(pc);
    case ST:
      return global2_predict(pc);
    default:
      printf("Undefined state in predictor table 2 ");
      return NOTTAKEN;
  }
}

// QUESTION: how does the ghr get limited to 12 bits...
void train_tour2(uint32_t pc, uint8_t outcome) {
  // train local
  train_gshare(pc, outcome); 
  // train global
  train_global2(pc, outcome);

  // train choice 
  uint32_t historyBits = 1 << globalhistoryBits2;
  uint32_t ghr_lower = ghr & (historyBits - 1);
  
  switch(cpredictors[ghr_lower]) {
    case SN:
      cpredictors[ghr_lower] = (outcome == TAKEN) ? WN : SN;
      break;
    case WN:
      cpredictors[ghr_lower] = (outcome == TAKEN) ? WT : SN;
      break;
    case WT:
      cpredictors[ghr_lower] = (outcome == TAKEN) ? ST : WN;
      break;
    case ST:
      cpredictors[ghr_lower] = (outcome == TAKEN) ? ST : WT;
      break;
    default:
      break;
  }
  //printf("choice training done\n");
  ghr = ((ghr << 1 ) | outcome);

}

void cleanup_tour2() {
  free(globalpredictors2);
  free(cpredictors);
  free(localpredictors);
  free(gpredictors);
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
      init_tour2();
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
      return tour2_predict(pc);
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
      return train_tour2(pc, outcome);
    default:
      break;
  }
  

}
