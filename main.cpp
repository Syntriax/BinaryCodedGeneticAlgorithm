#include <iostream>
#include <cstdlib>

#define ITERATION 100
#define POPULATIONCOUNT 20
#define CROSSOVERCHANCE .5
#define MUTATIONCHANCE .02
#define CROSSOVERCOUNTPERITERATION 10
#define POPULATIONSIZE 200
#define BITCOUNT 12

const float ValuePeriod = (float)(1 << BITCOUNT);
const unsigned int ValuePeriodMask = (1 << BITCOUNT) - 1;

struct Chromosome
{
    unsigned int x1;
    unsigned int x2;
};

unsigned int GetRandomValue() { return rand() & ValuePeriodMask; }
float GetRandomPercentage()
{
    // RAND_MAX in Windows causes the rand values to be between 0 and 32767
    // So to overcome we get two random numbers and bitshift the one of them
    int random = rand() << 15 | rand();
    return float(random % 1000000) / 1000000.0;
}

float GetX1(unsigned int value) { return -1.5 + value * (4.0 / ValuePeriod); }
float GetX2(unsigned int value) { return value * (5.0 / ValuePeriod); }

float GetFitnessScore(const Chromosome &chromosome)
{
    float x1 = GetX1(chromosome.x1);
    float x2 = GetX2(chromosome.x2);

    float score = 40.0;
    score -= 4.5 * x1;
    score += 4.0 * x2;
    score -= x1 * x1;
    score -= 2.0 * x2 * x2;
    score += 2.0 * x1 * x2;
    score -= x1 * x1 * x1 * x1;
    score += 2.0 * x1 * x1 * x2;
    return score;
}

void Initialize(Chromosome &chromosome)
{
    chromosome.x1 = GetRandomValue();
    chromosome.x2 = GetRandomValue();
}

unsigned int InvertBits(unsigned int &value) { return (~value) & ValuePeriodMask; }
unsigned int GetRandomBits(float chance)
{
    unsigned int result = 0;

    for (size_t i = 0; i < BITCOUNT; i++)
        if (GetRandomPercentage() < chance)
            result += 1 << i;

    return result & ValuePeriodMask;
}

void Mutate(Chromosome &chromosome)
{
    unsigned int mutationBitsX1 = GetRandomBits(MUTATIONCHANCE);
    unsigned int mutationBitsX2 = GetRandomBits(MUTATIONCHANCE);

    chromosome.x1 ^= mutationBitsX1;
    chromosome.x2 ^= mutationBitsX2;
}

Chromosome Crossover(Chromosome &chromosomeLeft, Chromosome &chromosomeRight)
{
    Chromosome result = Chromosome();

    unsigned int crossoverBitsX1 = GetRandomBits(CROSSOVERCHANCE);
    unsigned int crossoverBitsX2 = GetRandomBits(CROSSOVERCHANCE);

    result.x1 = chromosomeLeft.x1 & crossoverBitsX1;
    result.x1 |= chromosomeRight.x1 & InvertBits(crossoverBitsX1);

    result.x2 = chromosomeLeft.x2 & crossoverBitsX2;
    result.x2 |= chromosomeRight.x2 & InvertBits(crossoverBitsX2);

    return result;
}

void UpdateFitnessScores(Chromosome *populationPointer, float *fitnessPointer)
{
    for (size_t i = 0; i < POPULATIONSIZE; i++)
        *(fitnessPointer++) = GetFitnessScore(*(populationPointer++));
}

Chromosome *GetRandomChromosome(Chromosome *populationPointer) { return populationPointer + rand() % POPULATIONSIZE; }
Chromosome *GetRandomCumulativeChromosome(Chromosome *populationPointer, float *fitnessPointer)
{
    float sumOfScores = 0.0;
    float cumulativeFloat = 0.0;
    float randomPoint = GetRandomPercentage();
    size_t i;

    for (i = 0; i < POPULATIONSIZE; i++)
        sumOfScores += *(fitnessPointer + i);

    for (i = 0; i < POPULATIONSIZE; i++)
    {
        cumulativeFloat += *(fitnessPointer + i) / sumOfScores;

        if (randomPoint <= cumulativeFloat)
            return populationPointer + i;
    }

    return populationPointer;
}

Chromosome *GetFittest(Chromosome *populationPointer, float *fitnessPointer)
{
    unsigned int fittestIndex = 0;
    for (size_t i = 0; i < POPULATIONSIZE; i++)
        if (*(fitnessPointer + i) > *(fitnessPointer + fittestIndex))
            fittestIndex = i;

    return populationPointer + fittestIndex;
}

Chromosome *GetElitistOffSpring(Chromosome *fittest, Chromosome *populationPointer)
{
    Chromosome *offspring = nullptr;

    do
        offspring = GetRandomChromosome(populationPointer);
    while (fittest == offspring);

    return offspring;
}

void SortScores(float *scores)
{
    float temp;
    for (size_t i = 0; i < POPULATIONCOUNT; i++)
        for (size_t j = i + 1; j < POPULATIONCOUNT; j++)
            if (*(scores + i) > *(scores + j))
            {
                temp = *(scores + i);
                *(scores + i) = *(scores + j);
                *(scores + j) = temp;
            }
}

int main()
{
    Chromosome population[POPULATIONSIZE];
    float fitnessScores[POPULATIONSIZE];
    float fitnessScoreRecord[POPULATIONCOUNT];
    Chromosome *offspring = nullptr;
    Chromosome *left = nullptr;
    Chromosome *right = nullptr;
    Chromosome *fittest = nullptr;
    int i;
    int generation;
    int crossoverCounter;
    int mutationIndex;
    int populationCounter;

    srand(0);

    for (populationCounter = 0; populationCounter < POPULATIONCOUNT; populationCounter++)
    {
        for (i = 0; i < POPULATIONSIZE; i++)
            Initialize(population[i]);
        UpdateFitnessScores(population, fitnessScores);
        fittest = GetFittest(population, fitnessScores);

        for (generation = 0; generation < ITERATION; generation++)
        {
            for (crossoverCounter = 0; crossoverCounter < CROSSOVERCOUNTPERITERATION; crossoverCounter++)
            {
                offspring = GetElitistOffSpring(fittest, population);

                left = GetRandomCumulativeChromosome(population, fitnessScores);
                right = GetRandomCumulativeChromosome(population, fitnessScores);
                (*offspring) = Crossover(*left, *right);
            }

            // Mutate all except the Fittest
            for (mutationIndex = 0; mutationIndex < POPULATIONSIZE; mutationIndex++)
                if (population + mutationIndex != fittest)
                    Mutate(*(population + mutationIndex));

            UpdateFitnessScores(population, fitnessScores);
            fittest = GetFittest(population, fitnessScores);

            // std::cout << "Fittest Score: " << GetFitnessScore(*fittest) << "\n";
            // std::cout << "x1: " << GetX1(fittest->x1) << "\n";
            // std::cout << "x2: " << GetX2(fittest->x2) << "\n";
            // std::cout << "-----End of Generation " << generation << "-----\n";
        }
        fitnessScoreRecord[populationCounter] = GetFitnessScore(*fittest);
    }

    SortScores(fitnessScoreRecord);

    std::cout << "Population Size: " << POPULATIONSIZE << "\n";
    std::cout << "Population Count: " << POPULATIONCOUNT << "\n";
    std::cout << "Iteration Count: " << ITERATION << "\n";
    std::cout << "Crossover Per Iteration: " << CROSSOVERCOUNTPERITERATION << "\n";
    std::cout << "Crossover Ratio: " << CROSSOVERCHANCE << "\n";
    std::cout << "Mutation Ratio: " << MUTATIONCHANCE << "\n";
    std::cout << "Bits Per Value: " << BITCOUNT << "\n";
    std::cout << "-----------------\n";
    std::cout << "Best: " << fitnessScoreRecord[POPULATIONCOUNT - 1] << "\n";
    std::cout << "Median: " << fitnessScoreRecord[(POPULATIONCOUNT - 1) / 2] << "\n";
    std::cout << "Worst: " << fitnessScoreRecord[0] << "\n";

    return 0;
}
