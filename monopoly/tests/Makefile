CC = gcc
CFLAGS = -lm
FILES = test.c ../utils/dice.c ../utils/player.c ../utils/graphics/engine.c ../utils/inputHandler.c ../models/fields.c ../utils/graphics/animation.c ../config.h ../utils/fileReader.c ../utils/gameState.c ../utils/logger.c ../utils/helpers.c ../config.c ../models/keybinds.c
OBJ = $(FILES:.c=.o)
EXEC = solution

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c config.h
	$(CC) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
