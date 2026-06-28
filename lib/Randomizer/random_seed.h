#ifndef RANDOM_SEED_H
#define RANDOM_SEED_H

#define RANDOM_SEED_SAMPLES 16

template<uint8_t pin>
class RandomSeed
{
	public:
	void randomize(void);
};

template<uint8_t pin>
void RandomSeed<pin>::randomize(void){
  int seed = 0;
  while(seed == 0)
	for(uint8_t i = 0; i < RANDOM_SEED_SAMPLES; i++)
		seed = (seed << 1) ^ analogRead(pin);
  randomSeed(seed);
}

#endif
