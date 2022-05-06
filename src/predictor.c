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
int lhistoryBits = 10;


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
int *lpredictors; // local predictors
int *cpredictors; // choice predictors
int lHistoryTable; // local history table

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
// tournament functions
void init_tour(){

  // init global prediction table (12 bits)
  init_gshare();

  // init global history table (12 bits)

  // init local history table (10 bits)
  
  // initialize the choice prediction table (size 12 bits)
  int historyBits = 1 << ghistoryBits;
  cpredictors = (int*) malloc(historyBits * sizeof(int));

  for(int i = 0; i <= historyBits; i++) { // set WN to 2^10 states
    cpredictors[i] = WN;
  }

}

uint8_t tour_predict(uint32_t pc) {
  uint8_t globalPrediction; 
  uint8_t tourPrediction;
  uint8_t localPrediction;
  uint8_t choicePrediction;
  
  // conduct global history prediction

  // conduct 2-level local prediction
  
  // choice BHT prediction final mux
  int historyBits = 1 << ghistoryBits;
  int pc_lower_bits = pc & (historyBits - 1);
  int ghistory_lower = gHistoryTable & (historyBits - 1);
  int historyIndex = pc_lower_bits ^ (ghistory_lower);

  switch(gpredictors[historyIndex]) {
    case SN:
      return gshare_predict(pc);
    case WN:
      return gshare_predict(pc);
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Undefined state in predictor table 2 ");
      return NOTTAKEN;
  }

  return tourPrediction;
}

// QUESTION: how does the ghr get limited to 12 bits...
void train_tour(uint32_t pc, uint8_t outcome) {
  // train global history predictor
  train_gshare(pc, outcome);
  //printf("global training done \n\n");

  // train 2-level local

  // train choice 
  uint32_t historyBits = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (historyBits - 1);
  uint32_t ghistory_lower = gHistoryTable & (historyBits - 1);
  uint32_t historyIndex = pc_lower_bits ^ (ghistory_lower);
  
  switch(cpredictors[historyIndex]) {
    case SN:
      cpredictors[historyIndex] = (outcome == TAKEN) ? WN : SN;
      //printf("cpredictor => %u\n", cpredictors[historyIndex]);
      break;
    case WN:
      cpredictors[historyIndex] = (outcome == TAKEN) ? WT : SN;
      //printf("cpredictor => %u\n", cpredictors[historyIndex]);
      break;
    case WT:
      cpredictors[historyIndex] = (outcome == TAKEN) ? ST : WN;
      //printf("cpredictor => %u\n", cpredictors[historyIndex]);
      break;
    case ST:
      cpredictors[historyIndex] = (outcome == TAKEN) ? ST : WT;
      //printf("cpredictor => %u\n", cpredictors[historyIndex]);
      break;
    default:
      break;
  }
  //printf("choice training done\n");

}

void cleanup_tour() {
  free(gpredictors);
  free(cpredictors);
  free(lpredictors);
}

// custom predictor (probably TAGE)
// TODO 


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
      // TODO
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

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
    default:
      break;
  }
  

}
