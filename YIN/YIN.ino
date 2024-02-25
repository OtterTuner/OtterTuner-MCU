#define SAMPLING_RATE   16667
#define MIN_FREQUENCY   62
#define MAX_FREQUENCY   400
#define THRESHOLD       0.1
#define MAX_PERIOD      (SAMPLING_RATE / MIN_FREQUENCY)
#define BUFFER_LENGTH   1024

float differenceFunction[MAX_PERIOD];
float cumulativeMeanNormalizedDifference[MAX_PERIOD];

void computeAutocorrelation() {
    for( int tau = 0; tau < MAX_PERIOD; tau++ ) {
        differenceFunction[tau] = 0;
        for( int t = 0; t < signalLength; t++ ) {
            float difference = rawData[t] - rawData[t + tau];
            differenceFunction[tau] += difference * difference;
        }
    }
}

void computeCMND() {
    cumulativeMeanNormalizedDifference[0] = 1.0;
    float runningSum = 0;
    for( int tau = 1; tau < MAX_PERIOD; tau++ ) {
        runningSum += differenceFunction[tau];
        cumulativeMeanNormalizedDifference[tau] = differenceFunction[tau] * (tau/runningSum);
    }
}

// Function to find the fundamental frequency using YIN algorithm
float findPitch() {
    int bestPeriod = -1;
    float bestValue = 1.0;
    for (int tau = MIN_FREQUENCY * SAMPLING_RATE; tau < MAX_PERIOD; tau++) {
        if (cumulativeMeanNormalizedDifference[tau] < THRESHOLD) {
            if (cumulativeMeanNormalizedDifference[tau] < bestValue) {
                bestValue = cumulativeMeanNormalizedDifference[tau];
                bestPeriod = tau;
            }
        }
    }
    if (bestPeriod == -1) {
        return 0; // No pitch detected
    }
    return SAMPLING_RATE / bestPeriod; // Convert period to frequency
}

void setup() {
}

void loop() {
    for(int i = 0; i < BUFFER_LENGTH; i++) {
        rawData[i] = analogRead(A0);
    }

    computeAutocorrelation();
    computeCMND();
    float pitch = findPitch();
}
