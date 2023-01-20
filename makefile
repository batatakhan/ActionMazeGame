BUILD = build/

SOURCE = source/

OBJECTS := $(patsubst $(SOURCE)%.s, $(BUILD)%.o, $(wildcard $(SOURCE)*.s))
COBJECTS := $(patsubst $(SOURCE)%.c, $(BUILD)%.o, $(wildcard $(SOURCE)*.c))

myProg: $(OBJECTS) $(COBJECTS)
	gcc -lwiringPi -pthread -o myProg $(OBJECTS) $(COBJECTS)

$(BUILD)%.o: $(SOURCE)%.s
	as --gstabs -I $(SOURCE) $< -o $@

$(BUILD)%.o: $(SOURCE)%.c
	gcc -g -c -O0 -Wall -I $(SOURCE) $< -o $@

clean :
	-rm -f $(BUILD)*.o myProg
