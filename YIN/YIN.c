#include <stdio.h>
// #include "Guitar_C5.h"
#include "C4.h"

#define SAMPLING_RATE 2048
#define MIN_FREQUENCY 62
#define MAX_FREQUENCY 400
#define THRESHOLD 1.0
#define MAX_PERIOD (SAMPLING_RATE / MIN_FREQUENCY)

int signalLength = 1015;

float differenceFunction[MAX_PERIOD];
float cumulativeMeanNormalizedDifference[MAX_PERIOD];

void computeAutocorrelation() {
  for (int tau = 0; tau < MAX_PERIOD; tau++) {
    differenceFunction[tau] = 0;
    for (int t = 0; t < signalLength; t++) {
      float difference = rawData[t] - rawData[t + tau];
      differenceFunction[tau] += difference * difference;
    }
  }
}

void computeCMND() {
  cumulativeMeanNormalizedDifference[0] = 1.0;
  float runningSum = 0;
  for (int tau = 1; tau < MAX_PERIOD; tau++) {
    runningSum += differenceFunction[tau];
    cumulativeMeanNormalizedDifference[tau] =
        differenceFunction[tau] * (tau / runningSum);
  }
}

// Function to find the fundamental frequency using YIN algorithm
float findPitch() {
  int bestPeriod = -1;
  float bestValue = 1.0;
  printf("tau: %d\n", MIN_FREQUENCY * SAMPLING_RATE);
  for (int tau = SAMPLING_RATE / MAX_FREQUENCY; tau < MAX_PERIOD; tau++) {
    printf("cumulativeMeanNormalizedDifference[tau]: %f\n",
           cumulativeMeanNormalizedDifference[tau]);
    if (cumulativeMeanNormalizedDifference[tau] < THRESHOLD) {
      if (cumulativeMeanNormalizedDifference[tau] < bestValue) {
        bestValue = cumulativeMeanNormalizedDifference[tau];
        bestPeriod = tau;
      }
    }
  }
  printf("bestPeriod: %d\n", bestPeriod);
  if (bestPeriod == -1) {
    return 0; // No pitch detected
  }
  return SAMPLING_RATE / bestPeriod; // Convert period to frequency
}

int main(int argc, char **argv) {
  computeAutocorrelation();
  computeCMND();
  float pitch = findPitch();

  printf("pitch: %f\n", pitch);
}
