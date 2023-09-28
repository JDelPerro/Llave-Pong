#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>

// Dimensiones de la ventana
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// Dimensiones de las paletas y la pelota
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;
const int BALL_WIDTH = 20;
const int BALL_HEIGHT = 20;

// Velocidad de las paletas y la pelota
const int PADDLE_SPEED = 5;
const int BALL_SPEED = 5;

// Incremento de velocidad de la pelota con el tiempo
const int BALL_SPEED_INCREMENT = 1;

// Tiempo de juego en segundos
const int TIME = 80;

// Estructura para representar un evento del jue
typedef struct {
    int ballX;
    int ballY;
    int paddle1Y;
    int paddle2Y;
    int player1Score;
    int player2Score;
    int currentBallSpeed;
    Uint32 timeElapsed;
} GameEvent;

// Nodo de la lista enlazada de eventos
typedef struct Node {
    GameEvent data;
    struct Node* next;
} Node;

// Función para agregar un nuevo evento a la lista enlazada
Node* addEventToList(Node* head, GameEvent eventData) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Error al asignar memoria para un nuevo nodo.\n");
        exit(1);
    }

    newNode->data = eventData;
    newNode->next = head;

    printf("Event Added\n");

    return newNode;
}

// Función para liberar la memoria de la lista enlazada
void freeEventList(Node* head) {
    while (head != NULL) {
        Node* temp = head;
        head = head->next;
        free(temp);
    }
}

// Función para guardar la lista de eventos en un archivo
void saveEventListToFile(Node* head, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error al abrir el archivo para escritura: %s\n", filename);
        exit(1);
    }

    Node* current = head;
    while (current != NULL) {
        if (fwrite(&current->data, sizeof(GameEvent), 1, file) != 1) {
            fprintf(stderr, "Error al escribir en el archivo: %s\n", filename);
            exit(1);
        }
        current = current->next;
    }

    fclose(file);
    printf("Eventos guardados en '%s'\n", filename);
}

// Nueva función para cargar eventos desde un archivo
Node* loadEventListFromFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error al abrir el archivo para lectura: %s\n", filename);
        exit(1);
    }

    Node* head = NULL;
    GameEvent eventData;

    while (fread(&eventData, sizeof(GameEvent), 1, file) == 1) {
        head = addEventToList(head, eventData);
    }

    fclose(file);
    printf("Eventos cargados de '%s'\n", filename);

    return head;
}

int main(int argc, char* argv[]) {
    // Definir variables para el marcador de puntos de los jugadores
    int player1Score = 0;
    int player2Score = 0;

    // Inicialización de SDL2
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Error al inicializar SDL2: %s\n", SDL_GetError());
        return 1;
    }

    // Inicialización de SDL_ttf para manejar fuentes de texto
    if (TTF_Init() != 0) {
        fprintf(stderr, "Error al inicializar SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Creación de ventana y renderer
    SDL_Window* window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Creación de fuentes de texto para el temporizador y el contador de puntos
    TTF_Font* font = TTF_OpenFont("font.ttf", 36);
    SDL_Color textColor = { 255, 255, 255 };

    // Inicializar generador de números aleatorios con una semilla
    srand(time(NULL));

    // Posiciones iniciales de las paletas y la pelota
    int paddle1Y = (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2;
    int paddle2Y = (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2;
    int ballX = (WINDOW_WIDTH - BALL_WIDTH) / 2;
    int ballY = (WINDOW_HEIGHT - BALL_HEIGHT) / 2;

    // Direcciones iniciales de la pelota (aleatoria)
    int ballDX = (rand() % 2 == 0) ? BALL_SPEED : -BALL_SPEED;
    int ballDY = BALL_SPEED;

    // Tiempo de inicio del juego
    Uint32 startTime = SDL_GetTicks();

    // Velocidad de la pelota
    int currentBallSpeed = BALL_SPEED;

    // Variable para controlar el estado del juego
    bool inGame = false;

    // Variable para controlar la pantalla de inicio
    bool startScreen = true;

    // Variable para controlar la pantalla de Game Over
    bool gameOverScreen = false;

    // Variable para almacenar el ganador
    int winner = 0; // 0: Ningún ganador, 1: Jugador 1, 2: Jugador 2

    // Variable para controlar el tiempo de incremento de velocidad
    Uint32 lastSpeedIncrementTime = 0;

    // Lista enlazada para almacenar eventos del juego
    Node* eventList = NULL;

    // Bucle principal del juego
    bool quit = false;
    bool restartRequested = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            // Manejar eventos de teclado
            if (event.type == SDL_KEYDOWN) {
                if (startScreen) {
                    startScreen = false;
                    inGame = true;
                    startTime = SDL_GetTicks();
                }
                else if (gameOverScreen) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        // Reiniciar el juego
                        restartRequested = true;
                    }
                    else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        // Salir del juego
                        quit = true;
                    }
                }
            }
        }

        // Control de las paletas (solo cuando el juego está en marcha)
        if (inGame) {
            const Uint8* state = SDL_GetKeyboardState(NULL);
            if (state[SDL_SCANCODE_W] && paddle1Y > 0) {
                paddle1Y -= PADDLE_SPEED;
            }
            if (state[SDL_SCANCODE_S] && paddle1Y < WINDOW_HEIGHT - PADDLE_HEIGHT) {
                paddle1Y += PADDLE_SPEED;
            }
            if (state[SDL_SCANCODE_UP] && paddle2Y > 0) {
                paddle2Y -= PADDLE_SPEED;
            }
            if (state[SDL_SCANCODE_DOWN] && paddle2Y < WINDOW_HEIGHT - PADDLE_HEIGHT) {
                paddle2Y += PADDLE_SPEED;
            }
        }

        // Movimiento de la pelota (solo cuando el juego está en marcha)
        if (inGame) {
            ballX += ballDX;
            ballY += ballDY;
        }

        // Colisiones con las paletas
        if ((ballX < PADDLE_WIDTH) && (ballY + BALL_HEIGHT > paddle1Y) && (ballY < paddle1Y + PADDLE_HEIGHT)) {
            ballDX = currentBallSpeed;

            // Crear un nuevo evento de colisión de paleta 1 y agregarlo a la lista
            GameEvent collisionEvent;
            collisionEvent.ballX = ballX;
            collisionEvent.ballY = ballY;
            collisionEvent.paddle1Y = paddle1Y;
            collisionEvent.paddle2Y = paddle2Y;
            collisionEvent.player1Score = player1Score;
            collisionEvent.player2Score = player2Score;
            collisionEvent.currentBallSpeed = currentBallSpeed;
            collisionEvent.timeElapsed = SDL_GetTicks() - startTime;

            eventList = addEventToList(eventList, collisionEvent);
        }
        if ((ballX + BALL_WIDTH > WINDOW_WIDTH - PADDLE_WIDTH) && (ballY + BALL_HEIGHT > paddle2Y) && (ballY < paddle2Y + PADDLE_HEIGHT)) {
            ballDX = -currentBallSpeed;

            // Crear un nuevo evento de colisión de paleta 2 y agregarlo a la lista
            GameEvent collisionEvent;
            collisionEvent.ballX = ballX;
            collisionEvent.ballY = ballY;
            collisionEvent.paddle1Y = paddle1Y;
            collisionEvent.paddle2Y = paddle2Y;
            collisionEvent.player1Score = player1Score;
            collisionEvent.player2Score = player2Score;
            collisionEvent.currentBallSpeed = currentBallSpeed;
            collisionEvent.timeElapsed = SDL_GetTicks() - startTime;

            eventList = addEventToList(eventList, collisionEvent);
        }

        // Colisiones con los bordes
        if (ballY < 0 || ballY + BALL_HEIGHT > WINDOW_HEIGHT) {
            ballDY = -ballDY;
        }

        // Puntuación (continuación)
        if (ballX < 0) {
            // Jugador 2 anota
            player2Score++;

            // Crear un nuevo evento de anotación de jugador 2 y agregarlo a la lista
            GameEvent scoreEvent;
            scoreEvent.ballX = ballX;
            scoreEvent.ballY = ballY;
            scoreEvent.paddle1Y = paddle1Y;
            scoreEvent.paddle2Y = paddle2Y;
            scoreEvent.player1Score = player1Score;
            scoreEvent.player2Score = player2Score;
            scoreEvent.currentBallSpeed = currentBallSpeed;
            scoreEvent.timeElapsed = SDL_GetTicks() - startTime;

            eventList = addEventToList(eventList, scoreEvent);

            // Volver a colocar la pelota en el centro y cambiar la dirección aleatoriamente
            ballX = (WINDOW_WIDTH - BALL_WIDTH) / 2;
            ballY = (WINDOW_HEIGHT - BALL_HEIGHT) / 2;
            ballDX = (rand() % 2 == 0) ? BALL_SPEED : -BALL_SPEED;
            ballDY = BALL_SPEED;
            // Reiniciar la velocidad de la pelota
            currentBallSpeed = BALL_SPEED;
            // Actualizar el tiempo del último incremento de velocidad
            lastSpeedIncrementTime = SDL_GetTicks();
        }
        if (ballX + BALL_WIDTH > WINDOW_WIDTH) {
            // Jugador 1 anota
            player1Score++;

            // Crear un nuevo evento de anotación de jugador 1 y agregarlo a la lista
            GameEvent scoreEvent;
            scoreEvent.ballX = ballX;
            scoreEvent.ballY = ballY;
            scoreEvent.paddle1Y = paddle1Y;
            scoreEvent.paddle2Y = paddle2Y;
            scoreEvent.player1Score = player1Score;
            scoreEvent.player2Score = player2Score;
            scoreEvent.currentBallSpeed = currentBallSpeed;
            scoreEvent.timeElapsed = SDL_GetTicks() - startTime;

            eventList = addEventToList(eventList, scoreEvent);

            // Volver a colocar la pelota en el centro y cambiar la dirección aleatoriamente
            ballX = (WINDOW_WIDTH - BALL_WIDTH) / 2;
            ballY = (WINDOW_HEIGHT - BALL_HEIGHT) / 2;
            ballDX = (rand() % 2 == 0) ? BALL_SPEED : -BALL_SPEED;
            ballDY = BALL_SPEED;
            // Reiniciar la velocidad de la pelota
            currentBallSpeed = BALL_SPEED;
            // Actualizar el tiempo del último incremento de velocidad
            lastSpeedIncrementTime = SDL_GetTicks();
        }

        // Incremento de velocidad de la pelota cada segundo
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastSpeedIncrementTime >= 1000) {
            currentBallSpeed += BALL_SPEED_INCREMENT;
            lastSpeedIncrementTime = currentTime;
        }

        // Calcular el tiempo restante
        Uint32 elapsedTime = currentTime - startTime;
        Uint32 timeRemaining = (TIME * 1000) - elapsedTime;

        // Si el tiempo restante llega a cero, terminar el juego
        if ((timeRemaining <= 0 || timeRemaining > (TIME * 1000)) && !gameOverScreen) {
            inGame = false;
            gameOverScreen = true;
            if (player1Score > player2Score) {
                winner = 1;
            }
            else if (player2Score > player1Score) {
                winner = 2;
            }
            else {
                winner = 0; // Empate
            }

            // Guardar la lista de eventos en un archivo al finalizar el juego
            saveEventListToFile(eventList, "game_events.dat");
        }

        // Renderizado
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (restartRequested) {
            printf("Restarted\n");

            // Liberar la memoria de la lista de eventos actual
            freeEventList(eventList);

            // Cargar eventos desde el archivo
            eventList = loadEventListFromFile("game_events.dat");

            // Reiniciar las variables de juego con eventos cargados
            Node* currentEvent = eventList;
            while (currentEvent != NULL) {
                GameEvent eventData = currentEvent->data;

                // Actualizar las variables del juego con los datos cargados
                ballX = eventData.ballX;
                ballY = eventData.ballY;
                paddle1Y = eventData.paddle1Y;
                paddle2Y = eventData.paddle2Y;
                player1Score = eventData.player1Score;
                player2Score = eventData.player2Score;
                currentBallSpeed = eventData.currentBallSpeed;
                startTime = SDL_GetTicks() - eventData.timeElapsed;

                currentEvent = currentEvent->next;
            }

            // Reiniciar las banderas de control
            //inGame = true;
            //gameOverScreen = false;
            restartRequested = false;
        }

        if (startScreen) {
            // Pantalla de inicio: "Presione cualquier tecla para jugar"
            char startText[] = "Presione cualquier tecla para jugar";
            SDL_Surface* startSurface = TTF_RenderText_Solid(font, startText, textColor);
            SDL_Texture* startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
            SDL_Rect startRect;
            startRect.x = (WINDOW_WIDTH - startSurface->w) / 2;
            startRect.y = (WINDOW_HEIGHT - startSurface->h) / 2;
            startRect.w = startSurface->w;
            startRect.h = startSurface->h;
            SDL_RenderCopy(renderer, startTexture, NULL, &startRect);
            SDL_DestroyTexture(startTexture);
            SDL_FreeSurface(startSurface);
        }
        else if (gameOverScreen) {
            // Pantalla de Game Over
            char gameOverText[50];
            if (winner == 0) {
                snprintf(gameOverText, sizeof(gameOverText), "Empate - Puntos: %d", player1Score);
            }
            else {
                snprintf(gameOverText, sizeof(gameOverText), "Jugador %d gana - Puntos: %d", winner, (winner == 1) ? player1Score : player2Score);
            }

            SDL_Surface* gameOverSurface = TTF_RenderText_Solid(font, gameOverText, textColor);
            SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
            SDL_Rect gameOverRect;
            gameOverRect.x = (WINDOW_WIDTH - gameOverSurface->w) / 2;
            gameOverRect.y = (WINDOW_HEIGHT - gameOverSurface->h) / 2;
            gameOverRect.w = gameOverSurface->w;
            gameOverRect.h = gameOverSurface->h;
            SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
            SDL_DestroyTexture(gameOverTexture);
            SDL_FreeSurface(gameOverSurface);

            char pressAnyKeyText[] = "Presione cualquier tecla para salir";
            SDL_Surface* pressAnyKeySurface = TTF_RenderText_Solid(font, pressAnyKeyText, textColor);
            SDL_Texture* pressAnyKeyTexture = SDL_CreateTextureFromSurface(renderer, pressAnyKeySurface);
            SDL_Rect pressAnyKeyRect;
            pressAnyKeyRect.x = (WINDOW_WIDTH - pressAnyKeySurface->w) / 2;
            pressAnyKeyRect.y = gameOverRect.y + gameOverRect.h + 20;
            pressAnyKeyRect.w = pressAnyKeySurface->w;
            pressAnyKeyRect.h = pressAnyKeySurface->h;
            SDL_RenderCopy(renderer, pressAnyKeyTexture, NULL, &pressAnyKeyRect);
            SDL_DestroyTexture(pressAnyKeyTexture);
            SDL_FreeSurface(pressAnyKeySurface);
        }
        else {
            // Dibujar la línea de malla con menos opacidad que las paletas
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
            for (int y = 0; y < WINDOW_HEIGHT; y += 20) {
                SDL_RenderDrawLine(renderer, WINDOW_WIDTH / 2, y, WINDOW_WIDTH / 2, y + 10);
            }

            // Dibujar paletas y pelota si el juego está en marcha
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect paddle1Rect = { 0, paddle1Y, PADDLE_WIDTH, PADDLE_HEIGHT };
            SDL_Rect paddle2Rect = { WINDOW_WIDTH - PADDLE_WIDTH, paddle2Y, PADDLE_WIDTH, PADDLE_HEIGHT };
            SDL_Rect ballRect = { ballX, ballY, BALL_WIDTH, BALL_HEIGHT };
            SDL_RenderFillRect(renderer, &paddle1Rect);
            SDL_RenderFillRect(renderer, &paddle2Rect);
            SDL_RenderFillRect(renderer, &ballRect);

            // Dibujar temporizador en la esquina superior izquierda
            Uint32 elapsedTimeSeconds = elapsedTime / 1000;
            char timerText[50];
            int minutes = (timeRemaining / 1000) / 60;
            int seconds = (timeRemaining / 1000) % 60;
            snprintf(timerText, sizeof(timerText), "Tiempo: %02d:%02d", minutes, seconds);
            SDL_Surface* timerSurface = TTF_RenderText_Solid(font, timerText, textColor);
            SDL_Texture* timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface);
            SDL_Rect timerRect;
            timerRect.x = 20;
            timerRect.y = 20;
            timerRect.w = timerSurface->w;
            timerRect.h = timerSurface->h;
            SDL_RenderCopy(renderer, timerTexture, NULL, &timerRect);
            SDL_DestroyTexture(timerTexture);
            SDL_FreeSurface(timerSurface);

            // Dibujar contador de puntos en la esquina superior derecha
            char scoreText[50];
            snprintf(scoreText, sizeof(scoreText), "Score: %d - %d", player1Score, player2Score);
            SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText, textColor);
            SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
            SDL_Rect scoreRect;
            scoreRect.x = WINDOW_WIDTH - scoreSurface->w - 20;
            scoreRect.y = 20;
            scoreRect.w = scoreSurface->w;
            scoreRect.h = scoreSurface->h;
            SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
            SDL_DestroyTexture(scoreTexture);
            SDL_FreeSurface(scoreSurface);
        }
        // Actualizar la ventana
        SDL_RenderPresent(renderer);

        // Controlar la velocidad del juego (aproximadamente 60 FPS)
        SDL_Delay(16);
    }

    // Liberar la memoria de la lista de eventos
    freeEventList(eventList);

    // Liberar recursos y cerrar el juego
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}