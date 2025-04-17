CC = gcc
TARGET = realtime
SRC = main.c
INCLUDE = -I/opt/homebrew/opt/libsndfile/include -I/opt/homebrew/include
LIBS = -L/opt/homebrew/opt/libsndfile/lib -L/opt/homebrew/lib -lsndfile -lportaudio -lm
CFLAGS = -Wall -Wextra -std=c11

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(INCLUDE) $(LIBS)

clean:
	rm -f $(TARGET)