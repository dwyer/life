#include <SDL2/SDL.h>
#include <stdbool.h>
#include <assert.h>

#define POW_2 9
/* #define WRAP */

#define SCALE 1

#ifdef POW_2
#define SIZE 1 << POW_2
#else
#define SIZE 600
#endif

#define CURRENT_BOARD (boards[gen_num & 1])
#define NEXT_BOARD (boards[~gen_num & 1])

#define out_of_bounds(x, y) ((x) < 0 || (y) < 0 || (x) >= w || (y) >= h)
#define get_cell(board, x, y) (board[(y)][(x)])

#define set_cell(board, x, y, cell) ((board)[(y)][(x)] = (cell))

#if defined(WRAP) && defined(POW_2)
#define safe_get_cell(board, x, y) (board[(y) % h][(x) % w])
#else
#define safe_get_cell(board, x, y) (out_of_bounds((x), (y)) ? 0 : (board)[(y)][(x)])
#endif

typedef unsigned int dim_t;

const dim_t w = SIZE;
const dim_t h = SIZE;

typedef int board_t[h][w];

const int oob_cell = 0;
board_t boards[2];
board_t diff;
board_t *current_board = &boards[0];
board_t *next_board = &boards[1];
int gen_num = 0;

SDL_Color bg = {0, 0, 0, 255};
SDL_Color fg = {255, 0, 0, 255};

void panic(char *msg)
{
    fprintf(stderr, "panic: %s\n", msg);
    exit(1);
}

void update_cell(board_t from, board_t to, dim_t x, dim_t y)
{
    if (out_of_bounds(x, y))
        panic("OUT OF BOUNDS on update_cell\n");
    int n = safe_get_cell(from, x-1, y-1)
        + safe_get_cell(from, x, y-1)
        + safe_get_cell(from, x+1, y-1)
        + safe_get_cell(from, x-1, y)
        + safe_get_cell(from, x+1, y)
        + safe_get_cell(from, x-1, y+1)
        + safe_get_cell(from, x, y+1)
        + safe_get_cell(from, x+1, y+1);

    if (get_cell(from, x, y))
        set_cell(to, x, y, n == 2 || n == 3);
    else
        set_cell(to, x, y, n == 3);

/*     if (get_cell(from, x, y) && !get_cell(to, x, y)) */
/*         set_cell(diff, x, y, -1); */
/*     else if (!get_cell(from, x, y) && get_cell(to, x, y)) */
/*         set_cell(diff, x, y, 1); */
/*     else */
/*         set_cell(diff, x, y, 0); */
}

void update_board(board_t from, board_t to) {
    for (dim_t y = 0; y < h; ++y)
        for (dim_t x = 0; x < w; ++x)
            update_cell(from, to, x, y);
}

void random_board(board_t board)
{
    for (dim_t y = 0; y < h; ++y)
        for (dim_t x = 0; x < w; ++x)
            if (!(rand() % 8))
                set_cell(board, x, y, 1);
}

#define FOR_X(i) for (dim_t i = 0; i < w; ++i)
#define FOR_Y(i) for (dim_t i = 0; i < h; ++i)
#define FOR_XY(x, y) FOR_X(x) FOR_Y(y)

#define HLINE(board, y) FOR_X(_HLINE_X) set_cell((board), _HLINE_X, (y), 1)
#define VLINE(board, x) FOR_Y(_VLINE_Y) set_cell((board), (x), _VLINE_Y, 1)

void clear_board(board_t board)
{
    FOR_X(x)
        FOR_Y(y)
        set_cell(board, x, y, 0);
}

void draw_x(board_t board)
{
    FOR_X(x) set_cell(board, x, x, 1);
    FOR_X(x) set_cell(board, x, h-x-1, 1);
}

void draw_box(board_t board)
{
    HLINE(board, 0);
    HLINE(board, h-1);
    VLINE(board, 0);
    VLINE(board, w-1);
}

void draw_cross(board_t board)
{
    HLINE(board, h/2);
    VLINE(board, w/2);
}

void draw_hash(board_t board)
{
    HLINE(board, h/4);
    HLINE(board, h/4*3);
    VLINE(board, w/4);
    VLINE(board, w/4*3);
}

void update_fg(void)
{
    if (fg.r == 255 && fg.g < 255 && !fg.b) ++fg.g;
    if (fg.g == 255 && fg.r > 0 && !fg.b) --fg.r;
    if (fg.g == 255 && fg.b < 255 && !fg.r) ++fg.b;
    if (fg.b == 255 && fg.g > 0 && !fg.r) --fg.g;
    if (fg.b == 255 && fg.r < 255 && !fg.g) ++fg.r;
    if (fg.r == 255 && fg.b > 0 && !fg.g) --fg.b;

}

void render_cell(SDL_Renderer *renderer, dim_t x, dim_t y)
{
    if (SCALE > 1) {
        SDL_Rect rect = {x*SCALE, y*SCALE, SCALE, SCALE};
        SDL_RenderFillRect(renderer, &rect);
    } else {
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

void render_board(SDL_Renderer *renderer, board_t board)
{
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, 255);
    FOR_XY(x, y)
        if (get_cell(board, x, y))
            render_cell(renderer, x, y);
    SDL_RenderPresent(renderer);
}

void render_diff(SDL_Renderer *renderer, board_t board)
{
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    FOR_XY(x, y)
        if (get_cell(board, x, y) == -1)
            render_cell(renderer, x, y);
    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, 255);
    FOR_XY(x, y)
        if (get_cell(board, x, y) == 1)
            render_cell(renderer, x, y);
    SDL_RenderPresent(renderer);
    return;

    FOR_XY(x, y) {
        int cell = get_cell(board, x, y);
        if (!cell)
            continue;
        switch (cell) {
        case -1:
            SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
            break;
        case 1:
            SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, 255);
            break;
        default:
            panic("oh shit\n");
            break;
        }
        render_cell(renderer, x, y);
    }
    SDL_RenderPresent(renderer);
}

void load_life_file(FILE *fp, board_t board, dim_t x, dim_t y)
{
    dim_t dx = 0;
    dim_t dy = 0;
    bool in_comment = false;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        putchar(ch);
        if (in_comment) {
            in_comment = (ch != '\n');
            continue;
        }
        if (ch == '#') {
            in_comment = true;
            continue;
        }
        if (ch == '\n') {
            dx = 0;
            ++dy;
            continue;
        }
        if (ch == ' ' || ch == '.')
            set_cell(board, x + dx, y + dy, 0);
        else
            set_cell(board, x + dx, y + dy, 1);
        ++dx;
    }
}

void load_rle(FILE *fp, board_t board, dim_t x, dim_t y)
{
    int ch;
    bool in_comment = false;
    bool in_header = true;
    int run = 0;
    dim_t dx = 0;
    dim_t dy = 0;
    while ((ch = fgetc(fp)) != EOF) {
        if (in_comment) {
            in_comment = ch != '\n';
        } else if (ch == '#') {
            in_comment = true;
        } else if (in_header) {
            in_header = ch != '\n';
        } else if (isdigit(ch)) {
            run = run * 10 + ch - '0';
        } else if (ch == 'b' || ch == 'o' || ch == '$') {
            if (!run)
                run = 1;
            while (run) {
                if (ch == '$') {
                    dx = 0;
                    ++dy;
                } else {
                    set_cell(board, x+dx, y+dy, ch == 'o');
                    ++dx;
                }
                --run;
            }
        } else if (isspace(ch)) {
        } else if (ch == '!') {
            break;
        } else {
            panic("Couldn't parse RLE file.");
        }
    }
}

void copy_board(const board_t from, board_t to)
{
    FOR_XY(x, y) set_cell(to, x, y, get_cell(from, x, y));
}

void zoom(const board_t src, board_t dst)
{
    FOR_XY(dst_x, dst_y) {
        if (dst_x % 2 || dst_y % 2)
            continue;
        dim_t src_x = w / 4 + dst_x / 2;
        dim_t src_y = h / 4 + dst_y / 2;
        set_cell(dst, dst_x+0, dst_y+0, get_cell(src, src_x, src_y));
        set_cell(dst, dst_x+0, dst_y+1, get_cell(src, src_x, src_y));
        set_cell(dst, dst_x+1, dst_y+0, get_cell(src, src_x, src_y));
        set_cell(dst, dst_x+1, dst_y+1, get_cell(src, src_x, src_y));
    }
}

int main(int argc, char *argv[])
{
    bool pause = true;
    int delay = 10;
    Uint32 startTime = 0;
    int frame = 0;

    while (--argc) {
        ++argv;
        if (!strcmp(*argv, "--rle")) {
            assert(--argc);
            ++argv;
            FILE *fp = fopen(*argv, "r");
            assert(fp);
            load_rle(fp, CURRENT_BOARD, w/2, h/2);
            fclose(fp);
        } else if (!strcmp(*argv, "--life")) {
            assert(--argc);
            ++argv;
            FILE *fp = fopen(*argv, "r");
            assert(fp);
            load_life_file(fp, CURRENT_BOARD, w/2, h/2);
            fclose(fp);
        }
    }

    printf("size = (%d, %d)\n", w, h);

#ifdef POW_2
    printf("size is power of two (2^%d)\n", POW_2);
#endif

#ifdef WRAP
    printf("wrapping enabled\n");
#else
    printf("out of bounds cells are %s\n", oob_cell ? "alive" : "dead");
#endif

    printf("delay = %d\n", delay);
    srand(time(NULL));
    printf("getting ready %d\n", (unsigned)-1 % w);
    /* draw_cross(CURRENT_BOARD); */

    printf("ready\n");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Life",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            w * SCALE, h * SCALE,
            SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer(w * SCALE, h * SCALE, 0, &window, &renderer);
    startTime = SDL_GetTicks();
    while (1) {
        update_fg();
        render_board(renderer, CURRENT_BOARD);
        SDL_UpdateWindowSurface(window);
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                goto quit;
            /* if (e.type == SDL_MOUSEMOTION) { */
            /*     printf("%d, %d\n", e.motion.x, e.motion.y); */
            /*     set_cell(CURRENT_BOARD, e.motion.x, e.motion.y, 1); */
            /*     set_cell(CURRENT_BOARD, e.motion.x+1, e.motion.y, 1); */
            /*     set_cell(CURRENT_BOARD, e.motion.x-1, e.motion.y, 1); */
            /*     set_cell(CURRENT_BOARD, e.motion.x, e.motion.y+1, 1); */
            /*     set_cell(CURRENT_BOARD, e.motion.x, e.motion.y-1, 1); */
            /* } */
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_SPACE:
                    pause = !pause;
                    break;
                case SDLK_BACKSPACE:
                    clear_board(CURRENT_BOARD);
                    pause = true;
                    break;
                case SDLK_b:
                    draw_box(CURRENT_BOARD);
                    break;
                case SDLK_c:
                    draw_cross(CURRENT_BOARD);
                    break;
                case SDLK_h:
                    draw_hash(CURRENT_BOARD);
                    break;
                case SDLK_q:
                    goto quit;
                case SDLK_r:
                    random_board(CURRENT_BOARD);
                    break;
                case SDLK_x:
                    draw_x(CURRENT_BOARD);
                    break;
                case SDLK_z:
                    printf("zoom\n");
                    copy_board(CURRENT_BOARD, NEXT_BOARD);
                    zoom(NEXT_BOARD, CURRENT_BOARD);
                    break;
                }
            }
        }
        if (!pause) {
            update_board(CURRENT_BOARD, NEXT_BOARD);
            board_t *tmp = current_board;
            current_board = next_board;
            next_board = tmp;
            if (delay)
                SDL_Delay(delay);
            gen_num++;
        }
        frame = (frame + 1) % 60;
        if (!frame) {
            Uint32 newTime = SDL_GetTicks();
            float sec = (newTime - startTime) / 1000.0;
            fprintf(stderr, "%g fps (%g secs/60 frames)\n", 60 / sec, sec);
            startTime = newTime;
        }
    }
quit:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
