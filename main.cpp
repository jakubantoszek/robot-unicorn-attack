#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define Y_MAX_POSITION	370
#define UNICORN_WIDTH	80
#define UNICORN_HEIGHT	66
#define SCENE_WIDTH	    6400
#define OBSTACLE_NUM	10
#define FAIRY_NUM		5
#define FAIRY_WIDTH	    25
#define MAX_DASHES      3
#define FAIRY_FIELD_MIN	120
#define FAIRY_FIELD_MAX	220
#define STAR_NUM		3
#define LIFES			3
#define MIN_STAR_VALUE	100
#define MIN_FAIRY_VALUE 10
#define MIN_TEMPO		1

// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};

// funkcja zwraca wiêksz¹ z podanych wartoœci
int max(int a, int b)
{
	if (a >= b)
		return a;
	else return b;
}

// funkcja zwraca mniejsz¹ z podanych wartoœci
int min(int a, int b)
{
	if (a >= b)
		return b;
	else return a;
}

int random()
{
	int x = rand() % 10;
	if (x <= 6)
		return 1;
	else return 0;
}

int random_fairy_move()
{
	int x = rand() % 10;
	if (x <= 2)
		return -2;
	else if (x <= 4)
		return -4;
	else if (x <= 7)
		return 2;
	else return 4;
}

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void SetColors(int* black, int* green, int* red, int* blue, SDL_Surface* screen)
{
	*black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	*green = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	*red = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	*blue = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
}

// funkcja wyznacza aktualny stan gry
void GetGameTime(int* t1, double* game_time)
{
	int t2 = SDL_GetTicks();
	double delta = (t2 - *t1) * 0.001;
	*t1 = t2;
	*game_time += delta;
}

struct przeszkoda {
	int x; // wspó³rzêdna x przeszkody
	SDL_Surface* surf; // typ (numer) przeszkody
	int typ; // 0 - podstawowa przeszkoda, 1 - wy¿szy teren, 2 - stalaktyt
};

// struktura zawieraj¹ca wspó³rzêdne wró¿ek
struct fairy {
	int x;
	int y;
	int isSet;
};

przeszkoda* set_obstacles(SDL_Surface* screen)
{
	static przeszkoda przeszkody[OBSTACLE_NUM];
	SDL_Surface* surf;

	przeszkody[0].x = 400;
	przeszkody[0].surf = SDL_LoadBMP("./images/obstacle1.bmp");
	przeszkody[0].typ = 0;
	przeszkody[1].x = 600;
	przeszkody[1].surf = SDL_LoadBMP("./images/obstacle1.bmp");
	przeszkody[1].typ = 0;
	przeszkody[2].x = 1000;
	przeszkody[2].surf = SDL_LoadBMP("./images/obstacle2.bmp");
	przeszkody[2].typ = 1;
	przeszkody[3].x = 1500;
	przeszkody[3].surf = SDL_LoadBMP("./images/obstacle6.bmp");
	przeszkody[3].typ = 2;
	przeszkody[4].x = 1800;
	przeszkody[4].surf = SDL_LoadBMP("./images/obstacle1.bmp");
	przeszkody[4].typ = 0;
	przeszkody[5].x = 2300;
	przeszkody[5].surf = SDL_LoadBMP("./images/obstacle5.bmp");
	przeszkody[5].typ = 2;
	przeszkody[6].x = 2800;
	przeszkody[6].surf = SDL_LoadBMP("./images/obstacle1.bmp");
	przeszkody[6].typ = 0;
	przeszkody[7].x = 4000;
	przeszkody[7].surf = SDL_LoadBMP("./images/obstacle4.bmp");
	przeszkody[7].typ = 1;
	przeszkody[8].x = 4900;
	przeszkody[8].surf = SDL_LoadBMP("./images/obstacle1.bmp");
	przeszkody[8].typ = 0;
	przeszkody[9].x = 5400;
	przeszkody[9].surf = SDL_LoadBMP("./images/obstacle3.bmp");
	przeszkody[9].typ = 1;

	return przeszkody;
}

fairy* set_fairies()
{
	static fairy fairies[FAIRY_NUM]; // wspó³rzêdne wró¿ek ([0] - x, [1] - y) 

	fairies[0].x = 500;
	fairies[0].y = 170;
	fairies[1].x = 1900;
	fairies[1].y = 200;
	fairies[2].x = 3000;
	fairies[2].y = 160;
	fairies[3].x = 4300;
	fairies[3].y = 140;
	fairies[4].x = 5500;
	fairies[4].y = 130;

	return fairies;
}

int* set_stars() {
	static int stars[STAR_NUM];

	stars[0] = 2200;
	stars[1] = 3100;
	stars[2] = 6100;

	return stars;
}

void DrawObstacles(przeszkoda *przeszkody, SDL_Surface *scene)
{
	for (int i = 0; i < OBSTACLE_NUM; ++i)
	{
		if (przeszkody[i].typ == 2)
			DrawSurface(scene, przeszkody[i].surf, przeszkody[i].x + przeszkody[i].surf->w / 2,
				przeszkody[i].surf->h / 2);
		else DrawSurface(scene, przeszkody[i].surf, przeszkody[i].x + przeszkody[i].surf->w / 2,
			Y_MAX_POSITION - 3 - przeszkody[i].surf->h / 2);
	}
}

void DrawMenu(SDL_Surface* screen, SDL_Surface* start_image, SDL_Surface* charset)
{
	int czarny, zielony, czerwony, niebieski;
	char text[128];
	SetColors(&czarny, &zielony, &czerwony, &niebieski, screen);

	DrawSurface(screen, start_image,
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	sprintf(text, "Robot Unicorn Attack");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

	sprintf(text, "n - new game, Esc - exit");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
}

void DrawText(SDL_Surface* screen, SDL_Surface* charset, double game_time, int points)
{
	int czarny, zielony, czerwony, niebieski;
	char text[128];
	SetColors(&czarny, &zielony, &czerwony, &niebieski, screen);

	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	sprintf(text, "Czas trwania gry = %.1lf s", game_time);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

	sprintf(text, "Punkty = %d", points);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
}

void DrawBackground(SDL_Surface* screen, SDL_Surface* scene, int pos_x)
{
	if (pos_x % SCENE_WIDTH <= SCENE_WIDTH - SCREEN_WIDTH)
		DrawSurface(screen, scene,
			SCREEN_WIDTH * 5 - pos_x % SCENE_WIDTH, SCREEN_HEIGHT / 2); // wyœwietlanie t³a

	else { // rysowanie w przypadku zapêtlenia siê sceny 
		DrawSurface(screen, scene,
			SCREEN_WIDTH * 5 - pos_x % SCENE_WIDTH, SCREEN_HEIGHT / 2);
		SDL_Rect destRect;
		destRect.w = SCREEN_WIDTH;
		destRect.h = SCREEN_HEIGHT;
		destRect.x = max(0, SCREEN_WIDTH - (pos_x % SCREEN_WIDTH));
		destRect.y = 0;

		SDL_BlitSurface(scene, NULL, screen, &destRect);
	}
}

void DrawLifes(SDL_Surface* screen, SDL_Surface* life, SDL_Surface* lost_life, int lifes)
{
	for (int i = 1; i <= LIFES; ++i)
	{
		if (lifes >= i)
			DrawSurface(screen, life, life->w / 2 + 5 + (life->w + 5) * (i - 1), life->h / 2 + 45);
		else DrawSurface(screen, lost_life, lost_life->w / 2 + 5 + (lost_life->w + 5) * (i - 1),
			lost_life->h / 2 + 45);
	}
}

int SearchObstacle(int x, przeszkoda przeszkody[])
{
	for (int i = 0; i < OBSTACLE_NUM; ++i)
		if (x <= przeszkody[i].x + przeszkody[i].surf->w)
			return i;
	return 0;
}

void ChangeFairies(int pos_x, fairy* fairies, int *actual_fairy, int *fairy_checked, int *fairy_points, int *checked1,
	int *fairy_value, int *fairy_y, SDL_Surface *fairy, SDL_Surface *screen, int *fairy_move, SDL_Rect unicorn_rect)
{
	SDL_Rect fairy_rect;

	if (pos_x % SCENE_WIDTH > fairies[*actual_fairy].x + 25)
	{
		*actual_fairy = (*actual_fairy + 1) % FAIRY_NUM;
		if (*fairy_checked == 1)
		{
			*fairy_points += *fairy_value;
			*fairy_value += 10;
			*checked1 = 1;
			*fairy_checked = 0;
		}
		else if (*checked1 == 0)
			*fairy_value = 10;
	}
	else if ((pos_x % SCENE_WIDTH + SCREEN_WIDTH > fairies[*actual_fairy].x) && (fairies[*actual_fairy].isSet == 1)
		&& (*fairy_checked == 0))
	{
		*checked1 = 0;
		fairy_rect.x = fairies[*actual_fairy].x % SCENE_WIDTH;
		fairy_rect.y = *fairy_y - fairy->h / 2;
		fairy_rect.w = fairy->w;
		fairy_rect.h = fairy->h;

		if (SDL_HasIntersection(&unicorn_rect, &fairy_rect))
			*fairy_checked = 1;

		*fairy_move += random_fairy_move();
		*fairy_y = min(FAIRY_FIELD_MAX, max(FAIRY_FIELD_MIN,
			fairies[*actual_fairy].y + fairy->h / 2 + *fairy_move));

		DrawSurface(screen, fairy, fairies[*actual_fairy].x - pos_x % SCENE_WIDTH + fairy->w / 2,
			*fairy_y);
	}
	else {
		*fairy_move = 0;
	}
}

void ChangeStars(int *pos_x, int stars[STAR_NUM][2], int *actual_star, int *star_checked, int *star_points, 
	int *star_value, int *checked2, int *zryw, int *fail, SDL_Surface *star, SDL_Surface* screen, SDL_Rect unicorn_rect)
{
	SDL_Rect star_rect; 

	if (*pos_x % SCENE_WIDTH > stars[*actual_star][0] + 25)
	{
		*actual_star = (*actual_star + 1) % STAR_NUM;
		if (*star_checked == 1)
		{
			*star_points += *star_value;
			*star_value += 100;
			*checked2 = 1;
			*star_checked = 0;
		}
		else if (*checked2 == 0)
			*star_value = 100;
	}
	else if ((*pos_x % SCENE_WIDTH + SCREEN_WIDTH > stars[*actual_star][0]) && (stars[*actual_star][1] == 1)
		&& (*star_checked == 0))
	{
		*checked2 = 0;
		star_rect.x = stars[*actual_star][0] % SCENE_WIDTH + 5;
		star_rect.y = Y_MAX_POSITION - star->h + 5;
		star_rect.w = star->w - 5;
		star_rect.h = star->h;

		if (SDL_HasIntersection(&unicorn_rect, &star_rect))
		{
			if (*zryw == 0)
				*fail = 1;
			else *star_checked = 1;
		}

		DrawSurface(screen, star, stars[*actual_star][0] - *pos_x % SCENE_WIDTH + star->w / 2,
			Y_MAX_POSITION - star->h / 2);
	}
}

void jump(int *jump_speed, int *pos_y, int more_height, int *releases, int end_jump)
{
	if (*jump_speed > 1) // prêdkoœæ, je¿eli trzymana jest strza³ka w górê
	{
		if (*pos_y > Y_MAX_POSITION - (170 + more_height)) {
			*pos_y = max(0, *pos_y - *jump_speed);
		}
		else
		{
			*jump_speed = 0;
			*releases = -1;
		}
	}
	else if (*jump_speed == 1) // strza³ka w górê nie jest trzymana ale skok jest jeszcze wykonywany
	{
		if ((*pos_y > Y_MAX_POSITION - (170 + more_height)) && (*pos_y > end_jump - 50))
			*pos_y = max(0, *pos_y - 1);
		else *jump_speed = 0;
	}
	else if (*pos_y < Y_MAX_POSITION) // jednoro¿ec spada
		*pos_y = min(*pos_y + 1, Y_MAX_POSITION);
}

void JumpOnEvent(int *actual_jump, int *jump_speed, int *releases, int *more_height)
{
	if (*actual_jump == 0)
	{
		*actual_jump = 1;
		*jump_speed = 2;
	}
	else if (*actual_jump == 1)
	{
		if (*releases == 1)
		{
			*actual_jump = 2;
			*more_height = 20;
		}
		if (*releases != -1)
			*jump_speed = 2;
	}
	else *jump_speed = 0;
}

SDL_Rect GetUnicornRectangle(int pos_x, int pos_y)
{
	SDL_Rect unicorn_rect;

	unicorn_rect.x = 5 + pos_x % SCENE_WIDTH;
	unicorn_rect.y = pos_y - UNICORN_HEIGHT;
	unicorn_rect.w = UNICORN_WIDTH - 10;
	unicorn_rect.h = UNICORN_HEIGHT;

	return unicorn_rect;
}

SDL_Rect GetObstacleRectangle(przeszkoda *przeszkody, int next_obstacle)
{
	SDL_Rect obstacle_rect;

	obstacle_rect.x = przeszkody[next_obstacle].x;
	if (przeszkody[next_obstacle].typ == 2)
		obstacle_rect.y = 0;
	else obstacle_rect.y = (Y_MAX_POSITION - 3 - przeszkody[next_obstacle].surf->h);
	obstacle_rect.w = przeszkody[next_obstacle].surf->w - 8;
	obstacle_rect.h = przeszkody[next_obstacle].surf->h - 10;

	return obstacle_rect;
}

void ChangeMinHeight(przeszkoda *przeszkody, int next_obstacle, int pos_x, int *actual_obstacle, int *actual_height,
	int *pos_y, int previous_obstacle, int tempo)
{
	if (przeszkody[next_obstacle].typ == 1)
	{
		if (pos_x % SCENE_WIDTH + UNICORN_WIDTH - 10 - tempo >= przeszkody[next_obstacle].x)
		{
			*actual_obstacle = 1;
			*actual_height = przeszkody[next_obstacle].surf->h;
			*pos_y = min(*pos_y, Y_MAX_POSITION - *actual_height);
		}
		else {
			*actual_obstacle = 0;
			*actual_height = 0;
		}
	}
	else if ((pos_x % SCENE_WIDTH + 10 >= przeszkody[previous_obstacle].x) &&
		(pos_x % SCENE_WIDTH + 20 <= przeszkody[previous_obstacle].x + przeszkody[previous_obstacle].surf->w))
		{
			*actual_obstacle = 1;
			*actual_height = przeszkody[previous_obstacle].surf->h;
			*pos_y = min(*pos_y, Y_MAX_POSITION - *actual_height);
		}
	else 
	{
		*actual_obstacle = 0;
		actual_height = 0;
	}
}

void JumpKeyRelease(int *end_jump, int *jump_speed, int *releases, int pos_y)
{
	*end_jump = pos_y;
	if (*jump_speed > 1)
		*jump_speed = 1;
	if (*releases <= 0)
		*releases = 1;
}

void EndOfGame(SDL_Surface *screen, SDL_Surface *charset, int points, int suma_punktow, SDL_Texture *scrtex,
	SDL_Renderer *renderer)
{
	int czarny, zielony, czerwony, niebieski;
	char text[128];
	SetColors(&czarny, &zielony, &czerwony, &niebieski, screen);

	DrawRectangle(screen, SCREEN_WIDTH / 2 - 175, SCREEN_HEIGHT / 2 - 30,
		350, 52, czerwony, niebieski);
	sprintf(text, "Liczba punktow : %d", points);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 - 19, text, charset);
	sprintf(text, "Suma liczby punktow : %d", suma_punktow);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 - 5, text, charset);
	sprintf(text, "m - menu, esc - quit");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 + 11, text, charset);
	
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);

}

void EndOfLife(SDL_Surface* screen, SDL_Surface* charset, int points, SDL_Texture* scrtex, SDL_Renderer* renderer)
{
	int czarny, zielony, czerwony, niebieski;
	char text[128];
	SetColors(&czarny, &zielony, &czerwony, &niebieski, screen);

	DrawRectangle(screen, SCREEN_WIDTH / 2 - 175, SCREEN_HEIGHT / 2 - 20,
		350, 42, czerwony, niebieski);
	sprintf(text, "Liczba punktow : %d", points);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 - 10, text, charset);
	sprintf(text, "k - kontynuuj, m - menu");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 + 6, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit = 0, rc;
	int pos_x = 0, pos_y = Y_MAX_POSITION; // wspó³rzêdne x i y pozycji jednoro¿ca
	double delta, game_time = 0; // zmienne do liczenia i przechowywania czasu
	
	int checked1 = 0, checked2 = 0; // zmienne pomocnicze
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	int czarny, zielony, czerwony, niebieski;
	int wait = 0; // zmienna, która zmienia wartoœæ na 1 je¿eli czeka na odpowiedŸ u¿ytkownika

	int controls = 1; // 1 - sterowanie strza³kami, 2 - klasyczne sterowanie
	int zryw = 0; // stan zrywu jednoro¿ca
	int dashNumber = 0; // licznik zrywów przed dotkniêciem pod³o¿a
	
	int jump_speed = 0; // zmienna przyjmuje wiêksz¹ wartoœæ przy trzymaniu skoku
	int releases = 0; // zmienna zapamiêtuje czy klawisz jest puszczony (1) lub czy postaæ osi¹gnê³a maks. wysokoœæ (-1)
	int more_height = 0; // zwiêkszenie maksymalnej wysokoœci skoku (double jump)
	int actual_jump = 0; // zmienna pomocnicza do sprawdzania double jumpa
	int end_jump = 0; // wspó³rzêdna y unicorna w momencie w chwili puszczenia strzaz³ki
	
	int actual_loop = -1; // zmienna do losowania pojawienia siê wró¿ek i gwiazd
	int actual_fairy = 0;
	int fairy_move = 0; // oddalenie siê wró¿ki od swojego standardowego po³o¿enia
	int fairy_y = 0; // wspó³rzêdna y wró¿ki
	int fairy_points = 0; // zdobyte punkty za wró¿ki w ci¹gu ca³ej gry
	int fairy_value = MIN_FAIRY_VALUE; // punkty za dan¹ wró¿kê
	int fairy_checked = 0; // zmienna sprawdza czy wró¿ka zosta³a zebrana
	fairy* fairies; // tablica wspó³rzêdnych wró¿ek

	int stars[STAR_NUM][2]; // tablica mo¿liwych gwiazd na scenie
	int *stars_pom = { 0 };
	int star_points = 0; // zdobyte punkty za gwiazdy w ci¹gu ca³ej gry
	int star_value = MIN_STAR_VALUE; // punkty za dan¹ gwiazdê
	int star_checked = 0; // zmienna sprawdza czy gwiazda zosta³a zebrana
	int actual_star = 0;
	int fail = 0; // 1 - uderzono w gwiazdê bez dasha

	przeszkoda* przeszkody; // przeszkody
	int next_obstacle = 0; // indeks najbli¿szej przeszkody
	int previous_obstacle = 0;
	int actual_obstacle = 0; // aktualna przeszkoda
	int actual_height = 0; // wysokoœæ aktualnej przeszkody

	int isStarted = 0; // sprawdzenie czy gra jest rozpoczêta
	int tempo = MIN_TEMPO; // tempo biegu jednoro¿ca 
	int points = 0; 
	int suma_punktow = 0; // suma punktów na koniec gry
	int lifes = LIFES;

	SDL_Event event;
	SDL_Rect unicorn_rect, obstacle_rect; // zmienne do sprawdzania kolizji unicorna z przeszkod¹
	SDL_Rect fairy_rect, star_rect; // prostok¹ty zawieraj¹ce wró¿kê i gwiazdê
	
	// zdefiniowanie powierzchni wystêpuj¹cych w grze
	SDL_Surface *screen, *charset;
	SDL_Surface *background_image, *start_image, *unicorn;
	SDL_Surface *fairy;
	SDL_Surface *star;
	SDL_Surface *scene;
	SDL_Surface *life, *lost_life;
	
	SDL_Texture* scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	srand(time(NULL));

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe³noekranowy
	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(window, "Robot Unicorn Attack");

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);

	scene = SDL_CreateRGBSurface(0, SCREEN_WIDTH * 10, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazków wystêpuj¹cych w grze
	background_image = SDL_LoadBMP("./images/background.bmp");
	if (background_image == NULL) {
		printf("SDL_LoadBMP(background.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(background_image);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie t³a menu gry
	start_image = SDL_LoadBMP("./images/image.bmp");
	if (start_image == NULL) {
		printf("SDL_LoadBMP(image.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(start_image);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka jednoro¿ca
	unicorn = SDL_LoadBMP("./images/unicorn.bmp");
	if (unicorn == NULL) {
		printf("SDL_LoadBMP(unicorn.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(unicorn);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka wró¿ki
	fairy = SDL_LoadBMP("./images/fairy.bmp");
	if (fairy == NULL) {
		printf("SDL_LoadBMP(fairy.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(fairy);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka gwiazdy
	star = SDL_LoadBMP("./images/star.bmp");
	if (star == NULL) {
		printf("SDL_LoadBMP(star.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(star);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka ¿ycia
	life = SDL_LoadBMP("./images/life.bmp");
	if (life == NULL) {
		printf("SDL_LoadBMP(life.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(life);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka straconego ¿ycia
	lost_life = SDL_LoadBMP("./images/lost_life.bmp");
	if (lost_life == NULL) {
		printf("SDL_LoadBMP(lost_life.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(lost_life);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	SDL_SetColorKey(charset, true, 0x000000);
	char text[128];
	SetColors(&czarny, &zielony, &czerwony, &niebieski, screen);

	for (int i = 0; i < 10; ++i)
		DrawSurface(scene, background_image,
			SCREEN_WIDTH / 2 + SCREEN_WIDTH * i, SCREEN_HEIGHT / 2);
	
	// wczytanie przeszkód, wró¿ek i gwiazd
	przeszkody = set_obstacles(screen);
	fairies = set_fairies();
	stars_pom = set_stars();
	for (int i = 0; i < STAR_NUM; ++i)
	{
		stars[i][0] = stars_pom[i];
		stars[i][1] = 0;
	}

	DrawObstacles(przeszkody, scene);

	while (!quit) {
		t2 = SDL_GetTicks();

		if (isStarted == 0) 
		{
			// gra nie jest rozpoczêta -> wyœwietl menu gry
			DrawMenu(screen, start_image, charset);
			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		else {
			DrawBackground(screen, scene, pos_x);

			DrawSurface(screen, unicorn,
				UNICORN_WIDTH / 2 + 5, pos_y - UNICORN_HEIGHT / 2); // wyœwietlanie jednoro¿ca
			DrawLifes(screen, life, lost_life, lifes);

			if (pos_x / SCENE_WIDTH > actual_loop) // losowanie pojawienia siê wró¿ek i gwiazd
			{
				for (int i = 0; i < FAIRY_NUM; ++i)
					fairies[i].isSet = random();

				for (int i = 0; i < STAR_NUM; ++i)
					stars[i][1] = random();
				actual_loop++;
			}

			ChangeFairies(pos_x, fairies, &actual_fairy, &fairy_checked, &fairy_points, &checked1, &fairy_value,
				&fairy_y, fairy, screen, &fairy_move, unicorn_rect);

			ChangeStars(&pos_x, stars, &actual_star, &star_checked, &star_points, &star_value, &checked2, &zryw,
				&fail, star, screen, unicorn_rect);

			points = pos_x / 4 + fairy_points + star_points;
			GetGameTime(&t1, &game_time);
			tempo = MIN_TEMPO + game_time / 30;

			DrawText(screen, charset, game_time, points);
			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);

			if((controls == 1) && (keys[SDL_SCANCODE_RIGHT]))
					pos_x += tempo;
		}

		// obs³uga zdarzeñ (o ile jakieœ zasz³y)
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_KEYDOWN:
				if ((isStarted == 1) && (zryw == 0))
				{
					if (controls == 1)
					{
						if (event.key.keysym.sym == SDLK_UP)
							JumpOnEvent(&actual_jump, &jump_speed, &releases, &more_height);

						if (event.key.keysym.sym == SDLK_DOWN)
							pos_y = min(pos_y + 3, Y_MAX_POSITION);

						if (event.key.keysym.sym == SDLK_d)
							controls = 2;
					}

					else {
						if (event.key.keysym.sym == SDLK_z)
							JumpOnEvent(&actual_jump, &jump_speed, &releases, &more_height);

						else if (event.key.keysym.sym == SDLK_d) // zmiana sterowania
							controls = 1;
					}
				}
				if (event.key.keysym.sym == SDLK_ESCAPE)
					quit = 1;
				break;
			case SDL_KEYUP:
				if ((zryw == 0) && (isStarted == 1))
				{
					if (((event.key.keysym.sym == SDLK_UP) && (controls == 1)) ||
						((event.key.keysym.sym == SDLK_z) && (controls == 2)))
						JumpKeyRelease(&end_jump, &jump_speed, &releases, pos_y);
					else if ((event.key.keysym.sym == SDLK_x) && (controls == 2) && (dashNumber < MAX_DASHES)) // dash
						zryw = 1;
				}
				break;
			};
			
			if (isStarted == 0)
			{
				if (keys[SDL_SCANCODE_N]) { // resetuj wszystkie zmienne
					t1 = SDL_GetTicks();
					isStarted = 1;
					lifes = LIFES;
					game_time = 0;
					controls = 1;
					points = 0;
					suma_punktow = 0;
					pos_y = Y_MAX_POSITION;
					zryw = 0;
					actual_loop = -1;
					actual_fairy = 0;
					fairy_points = 0;
					fairy_value = MIN_FAIRY_VALUE;
					star_points = 0;
					star_value = MIN_STAR_VALUE;
					actual_star = 0;
					tempo = MIN_TEMPO;
					jump_speed = 0;
				}
			}
		};
		
		if (isStarted == 1)
		{
			if (zryw == 0)
			{
				if (controls == 2)
					pos_x += tempo;

				jump(&jump_speed, &pos_y, more_height, &releases, end_jump);

				if (pos_y == Y_MAX_POSITION)
				{
					// reset zmiennych
					actual_jump = 0;
					end_jump = 0;
					releases = 0;
					more_height = 0;
					dashNumber = 0;
				}
			}
			else if ((zryw > 0) && (zryw < 50))
			{
				zryw++;
				pos_x += tempo * 2;
			}
			else if (zryw == 50) {
				zryw = 0;
				dashNumber++;
				if (actual_jump > 0)
					actual_jump = 1;
			}

			next_obstacle = SearchObstacle(pos_x % SCENE_WIDTH, przeszkody);

			// utworzenie prostok¹tów oznaczaj¹cych przeszkodê i jednoro¿ca w celu sprawdzenia kolizji
			unicorn_rect = GetUnicornRectangle(pos_x, pos_y);
			obstacle_rect = GetObstacleRectangle(przeszkody, next_obstacle);

			if (next_obstacle == 0)
				previous_obstacle = OBSTACLE_NUM - 1;
			else previous_obstacle = next_obstacle - 1;

			ChangeMinHeight(przeszkody, next_obstacle, pos_x, &actual_obstacle, &actual_height, &pos_y,
				previous_obstacle, tempo);

			// sprawdzenie czy nast¹pi³a kolizja
			if (((SDL_HasIntersection(&unicorn_rect, &obstacle_rect)) && (actual_obstacle == 0)) || (fail == 1))
			{
				pos_x = 0;
				lifes--;
				suma_punktow += points;
				wait = 1;
				fail = 0;
			}

			if (lifes == 0)
			{
				while (wait == 1)
				{
					EndOfGame(screen, charset, points, suma_punktow, scrtex, renderer);
					while (SDL_PollEvent(&event)) {
						switch (event.type) {
						case SDL_QUIT:
							quit = 1;
							break;
						case SDL_KEYDOWN:
							if (event.key.keysym.sym == SDLK_m)
							{
								wait = 0;
								isStarted = 0;
								pos_y = Y_MAX_POSITION;
								pos_x = 0;
							}
							else if (event.key.keysym.sym == SDLK_ESCAPE)
							{
								wait = 0;
								quit = 1;
							}
							break;
						}
					}
				}
			}

			else while (wait == 1)
			{
				EndOfLife(screen, charset, points, scrtex, renderer);
				while (SDL_PollEvent(&event)) {
					switch (event.type) {
					case SDL_QUIT:
						quit = 1;
						break;
					case SDL_KEYDOWN:
						if (event.key.keysym.sym == SDLK_k)
						{
							// reset zmiennych
							wait = 0;
							pos_y = Y_MAX_POSITION;
							zryw = 0;
							actual_loop = -1;
							actual_fairy = 0;
							fairy_points = 0;
							fairy_value = MIN_FAIRY_VALUE;
							star_points = 0;
							star_value = MIN_STAR_VALUE;
							actual_star = 0;
							tempo = MIN_TEMPO;
							game_time = 0;
							jump_speed = 0;
						}
						if (event.key.keysym.sym == SDLK_m)
						{
							wait = 0;
							isStarted = 0;
						}
						if (event.key.keysym.sym == SDLK_ESCAPE)
						{
							wait = 0;
							quit = 1;
						}
						break;
					}
				}
			}
		}
	};

	// zwolnienie powierzchni
	SDL_FreeSurface(background_image);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(charset);
	SDL_FreeSurface(start_image);
	SDL_FreeSurface(unicorn);
	SDL_FreeSurface(fairy);
	SDL_FreeSurface(star);
	SDL_FreeSurface(life);
	SDL_FreeSurface(lost_life);
	
	for(int i=0; i<OBSTACLE_NUM; ++i)
		SDL_FreeSurface(przeszkody[i].surf);
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
