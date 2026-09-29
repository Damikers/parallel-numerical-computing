#include <SDL3/SDL.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread/qos.h>
#include <time.h>

double time_left = 0.0;
double time_top  = 0.0;
int sleep_time = 25000;

#define WIDTH 800
#define HEIGHT 800
#define STEP 1

typedef struct { int x, y; } Particle;

Particle particle;
qos_class_t qos_left = QOS_CLASS_BACKGROUND;
qos_class_t qos_top  = QOS_CLASS_BACKGROUND;
bool running = true;
bool cube_out = false;
int moves_left = 0;
int moves_top = 0;

pthread_mutex_t lock;
pthread_t t1, t2;

// Return particle to the center
void reset_particle(void) {
    pthread_mutex_lock(&lock);
    particle.x = WIDTH / 2;
    particle.y = HEIGHT / 2;
    cube_out = false;
    pthread_mutex_unlock(&lock);
}

// Create thread with QoS
void create_thread_with_qos(pthread_t *thread, void *(*func)(void*), qos_class_t qos) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_set_qos_class_np(&attr, qos, 0);
    pthread_create(thread, &attr, func, NULL);
    pthread_attr_destroy(&attr);
}

// Threads
void* magnet_left(void* arg) {
    (void)arg;
    struct timespec start, end;
    while (true) {
        if (!cube_out) {
            clock_gettime(CLOCK_MONOTONIC, &start);

            pthread_mutex_lock(&lock);
            particle.x += STEP;
            moves_left++;
            printf("Left thread moved cube: x=%d\n", particle.x);
            fflush(stdout);
            pthread_mutex_unlock(&lock);

            clock_gettime(CLOCK_MONOTONIC, &end);
            time_left += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

            usleep(sleep_time);
        } else usleep(100000);
    }
    return NULL;
}

void* magnet_top(void* arg) {
    (void)arg;
    struct timespec start, end;
    while (true) {
        if (!cube_out) {
            clock_gettime(CLOCK_MONOTONIC, &start);

            pthread_mutex_lock(&lock);
            particle.y += STEP;
            moves_top++;
            printf("Top thread moved cube: y=%d\n", particle.y);
            fflush(stdout);
            pthread_mutex_unlock(&lock);

            clock_gettime(CLOCK_MONOTONIC, &end);
            time_top += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

            usleep(sleep_time);
        } else usleep(100000);
    }
    return NULL;
}

void check_priority_buttons(int mx, int my) {
    
    // Left magnet buttons
    SDL_Rect btn1 = {60, HEIGHT/2 - 80, 30, 30};
    SDL_Rect btn2 = {60, HEIGHT/2 - 40, 30, 30};
    SDL_Rect btn3 = {60, HEIGHT/2,      30, 30};
    if (mx >= btn1.x && mx <= btn1.x+btn1.w && my >= btn1.y && my <= btn1.y+btn1.h) {
        qos_left = QOS_CLASS_BACKGROUND;
        pthread_cancel(t1);
        create_thread_with_qos(&t1, magnet_left, qos_left);
    }
    if (mx >= btn2.x && mx <= btn2.x+btn2.w && my >= btn2.y && my <= btn2.y+btn2.h) {
        qos_left = QOS_CLASS_UTILITY;
        pthread_cancel(t1);
        create_thread_with_qos(&t1, magnet_left, qos_left);
    }
    if (mx >= btn3.x && mx <= btn3.x+btn3.w && my >= btn3.y && my <= btn3.y+btn3.h) {
        qos_left = QOS_CLASS_USER_INTERACTIVE;
        pthread_cancel(t1);
        create_thread_with_qos(&t1, magnet_left, qos_left);
    }

    // Top magnet buttons
    SDL_Rect btn4 = {WIDTH/2 + 60, 60, 30, 30};
    SDL_Rect btn5 = {WIDTH/2 + 100, 60, 30, 30};
    SDL_Rect btn6 = {WIDTH/2 + 140, 60, 30, 30};
    if (mx >= btn4.x && mx <= btn4.x+btn4.w && my >= btn4.y && my <= btn4.y+btn4.h) {
        qos_top = QOS_CLASS_BACKGROUND;
        pthread_cancel(t2);
        create_thread_with_qos(&t2, magnet_top, qos_top);
    }
    if (mx >= btn5.x && mx <= btn5.x+btn5.w && my >= btn5.y && my <= btn5.y+btn5.h) {
        qos_top = QOS_CLASS_UTILITY;
        pthread_cancel(t2);
        create_thread_with_qos(&t2, magnet_top, qos_top);
    }
    if (mx >= btn6.x && mx <= btn6.x+btn6.w && my >= btn6.y && my <= btn6.y+btn6.h) {
        qos_top = QOS_CLASS_USER_INTERACTIVE;
        pthread_cancel(t2);
        create_thread_with_qos(&t2, magnet_top, qos_top);
    }
}

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Magnets",
                                          WIDTH, HEIGHT,
                                          SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    pthread_mutex_init(&lock, NULL);

    reset_particle();
    create_thread_with_qos(&t1, magnet_left, qos_left);
    create_thread_with_qos(&t2, magnet_top, qos_top);

    SDL_Event e;
    SDL_Rect reset_btn = {350, 740, 100, 40};

    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) goto cleanup;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_R) reset_particle();
            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                // Reset
                if (mx >= reset_btn.x && mx <= reset_btn.x + reset_btn.w &&
                    my >= reset_btn.y && my <= reset_btn.y + reset_btn.h) reset_particle();

                check_priority_buttons(mx, my);
            }
        }

        pthread_mutex_lock(&lock);

        if (!cube_out &&
            (particle.x < 50 || particle.x > WIDTH-60 ||
             particle.y < 50 || particle.y > HEIGHT-60)) {
            cube_out = true;
            printf("=== Summary of moves and time ===\n");
            printf("Left thread moves: %d, active time: %.6f sec\n", moves_left, time_left);
            printf("Top thread moves: %d, active time: %.6f sec\n", moves_top, time_top);

            moves_left = 0;
            moves_top = 0;
            time_left = 0.0;
            time_top = 0.0;
            printf("Cube has left the table! Press Reset.\n");
        }

        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 200,200,200,255);
        SDL_FRect table1 = {50.0f, 50.0f, (float)(WIDTH-100), (float)(HEIGHT-100)};
        SDL_RenderRect(renderer, &table1);

        SDL_FRect table2 = {150.0f, 150.0f, (float)(WIDTH-300), (float)(HEIGHT-300)};
        SDL_RenderRect(renderer, &table2);

        SDL_FRect table3 = {250.0f, 250.0f, (float)(WIDTH-500), (float)(HEIGHT-500)};
        SDL_RenderRect(renderer, &table3);

        // Cube
        SDL_SetRenderDrawColor(renderer, 0,0,255,255);
        SDL_FRect p = {(float)particle.x, (float)particle.y, 10.0f, 10.0f};
        SDL_RenderFillRect(renderer, &p);

        // Magnets
        SDL_SetRenderDrawColor(renderer, 255,0,0,255);
        SDL_FRect mag_left_red = {20.0f, 400.0f, 30.0f, 250.0f};
        SDL_FRect mag_top_red  = {150.0f, 20.0f, 250.0f, 30.0f};
        SDL_RenderFillRect(renderer, &mag_left_red);
        SDL_RenderFillRect(renderer, &mag_top_red);

        SDL_SetRenderDrawColor(renderer, 0,0,255,255);
        SDL_FRect mag_left_blue = {20.0f, 150.0f, 30.0f, 250.0f};
        SDL_FRect mag_top_blue  = {400.0f, 20.0f, 250.0f, 30.0f};
        SDL_RenderFillRect(renderer, &mag_left_blue);
        SDL_RenderFillRect(renderer, &mag_top_blue);

        // Priority buttons
        SDL_SetRenderDrawColor(renderer, 0,180,0,255);
        SDL_FRect prio_btns[] = {
            // Left
            {60.0f, HEIGHT/2 - 80.0f, 30.0f,30.0f}, 
            {60.0f, HEIGHT/2 - 40.0f, 30.0f,30.0f}, 
            {60.0f, HEIGHT/2,          30.0f,30.0f},

            // Top
            {WIDTH/2 + 60.0f, 60.0f, 30.0f,30.0f}, 
            {WIDTH/2 + 100.0f, 60.0f, 30.0f,30.0f}, 
            {WIDTH/2 + 140.0f, 60.0f, 30.0f,30.0f}
        };
        for(int i=0;i<6;i++) SDL_RenderFillRect(renderer,&prio_btns[i]);

        // Black dots
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        for (int i=0;i<6;i++) {
            int numDots = (i % 3) + 1;
            for (int d=0; d<numDots; d++) {
                SDL_FRect dot = {
                    prio_btns[i].x + 6.0f + d*8.0f,
                    prio_btns[i].y + 12.0f,
                    5.0f, 5.0f
                };
                SDL_RenderFillRect(renderer, &dot);
            }
        }

        // White dots
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);

        // Left
        int left_index = 0;
        if (qos_left == QOS_CLASS_BACKGROUND) left_index = 0;
        if (qos_left == QOS_CLASS_UTILITY)    left_index = 1;
        if (qos_left == QOS_CLASS_USER_INTERACTIVE) left_index = 2;

        SDL_FRect dotL = {
            prio_btns[left_index].x + 12.0f,
            prio_btns[left_index].y + 20.0f,
            6.0f, 6.0f
        };
        SDL_RenderFillRect(renderer, &dotL);

        // Top
        int top_index = 3;
        if (qos_top == QOS_CLASS_BACKGROUND) top_index = 3;
        if (qos_top == QOS_CLASS_UTILITY)    top_index = 4;
        if (qos_top == QOS_CLASS_USER_INTERACTIVE) top_index = 5;

        SDL_FRect dotT = {
            prio_btns[top_index].x + 12.0f,
            prio_btns[top_index].y + 20.0f,
            6.0f, 6.0f
        };
        SDL_RenderFillRect(renderer, &dotT);

        // Reset
        SDL_SetRenderDrawColor(renderer, 180,0,0,255);
        SDL_FRect reset_btn_rect = {350.0f, 740.0f, 100.0f, 40.0f};
        SDL_RenderFillRect(renderer, &reset_btn_rect);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderRect(renderer, &reset_btn_rect);

        SDL_RenderPresent(renderer);
        pthread_mutex_unlock(&lock);

        SDL_Delay(16);

    }

cleanup:
    pthread_cancel(t1);
    pthread_cancel(t2);
    pthread_mutex_destroy(&lock);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
