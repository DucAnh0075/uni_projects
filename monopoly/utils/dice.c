#include "dice.h"

int rollDice() { return (rand() % 6) + 1; }

bool is_pash(int d1, int d2) {
  if (d1 != d2)
    return false;
  return d1 == d2;
}

/*void rollloop(){
      int diceValue, diceValue2;
    bool pash;
    int countpash = 0;
    do {
        printf("Press Enter to roll...");
        getchar();
        diceValue = rollDice();
        diceValue2 = rollDice();

        pash = is_pash(diceValue,diceValue2);
        if(pash){
          countpash++;
          printf("Number of pashes: %d\n",countpash);
        }
        printf("You rolled: %d, %d\n", diceValue, diceValue2);

    } while(pash && countpash < 3);
}
*/
