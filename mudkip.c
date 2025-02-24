#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define  M_UP    0
#define  M_DN    1
#define  M_LF    2
#define  M_RT    3

struct missile 
{
	SDL_Surface    *surface;
	SDL_Rect        box;
	SDL_Rect        pos;
	int             m_dir;
	int             m_go;
	struct missile *next;
};
typedef struct missile Missile;

int SCREEN_WIDTH;
int SCREEN_HEIGHT;

int main(int argc, char **argv)
{
	Missile       *mlist      = NULL;
	Missile       *mtmp       = NULL;
	Missile       *mtmp2      = NULL;

    SDL_Window    *window     = NULL;
    SDL_Surface   *screen     = NULL;
    SDL_Surface   *player     = NULL;
	SDL_Surface   *missile    = NULL;
    SDL_Surface   *block      = NULL;
    SDL_Joystick **pad        = NULL;
    SDL_Event      e;

    SDL_Rect       playerbox;  // player  sprite coords
    SDL_Rect       playerpos;  // player  screen position
	SDL_Rect       missilebox;
	SDL_Rect       missilepos;
    SDL_Rect       blockbox;   // block   sprite coords
    SDL_Rect       blockpos;   // block   screen position

    int            DEAD_ZONE  =  1;
    int            x          =  0;
	int            rfs        =  0;
    int            num_pads   =  0;
    int            frame      =  0;
    int            quit       =  0;
    int            p_go       =  5;
	int            p_dir      = M_DN; // player  direction

    Uint32         rmask;
    Uint32         gmask;
    Uint32         bmask;
    Uint32         amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask                     = 0xff000000;
    gmask                     = 0x00ff0000;
    bmask                     = 0x0000ff00;
    amask                     = 0x000000ff;
#else
    rmask                     = 0x000000ff;
    gmask                     = 0x0000ff00;
    bmask                     = 0x00ff0000;
    amask                     = 0xff000000;
#endif

    if (argc                 >= 3)
    {
        SCREEN_WIDTH          = atoi(argv[1]);
        SCREEN_HEIGHT         = atoi(argv[2]);
    }
    else
    {
        SCREEN_WIDTH          = 800;
        SCREEN_HEIGHT         = 600;
    }

    //////////////////////////////////////////////////////////////////
    //
    // set up player sprite on-screen location and dimensions
    //
    playerpos.w               = 36;
    playerpos.h               = 58;
    playerpos.x               = (SCREEN_WIDTH  / 2) - (playerpos.w / 2);
    playerpos.y               = (SCREEN_HEIGHT / 2) - (playerpos.h / 2);

    //////////////////////////////////////////////////////////////////
    //
    // set up player sprite dimensions (clipping from sprite sheet)
    //
    playerbox.x               = 0;
    playerbox.y               = 0;
    playerbox.w               = playerpos.w;
    playerbox.h               = playerpos.h;

    //////////////////////////////////////////////////////////////////
    //
    // set up missile sprite dimensions
    //
    missilebox.x              = 0;
    missilebox.y              = 0;
    missilebox.w              = 16;
    missilebox.h              = 16;

    //////////////////////////////////////////////////////////////////
    //
    // set up wall block dimensions
    //
    blockbox.x                = 0;
    blockbox.y                = 0;
    blockbox.w                = 40;
    blockbox.h                = 40;

    if (SDL_Init(SDL_INIT_EVERYTHING) <  0)
    {
        fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    num_pads                  = SDL_NumJoysticks();
    if (num_pads             <  1)
    {
        fprintf(stderr, "Warning: No joysticks connected!\n");
    }
    else
    {
        fprintf(stdout, "%d joysticks detected\n", num_pads);
        pad                   = (SDL_Joystick **) malloc(sizeof(SDL_Joystick *) * num_pads);
        for (x = 0; x < num_pads; x++)
        {
            *(pad+x)          = SDL_JoystickOpen(x);
            if (*(pad+x)     == NULL)
            {
                fprintf(stderr, "Warning: Unable to open game controller %d!\n", x);
                fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
            }
        }
    }

    window                    = SDL_CreateWindow("What is this?!?",
                                                 SDL_WINDOWPOS_UNDEFINED,
                                                 SDL_WINDOWPOS_UNDEFINED,
                                                 SCREEN_WIDTH,
                                                 SCREEN_HEIGHT,
                                                 SDL_WINDOW_SHOWN);

    if (window               == NULL)
    {
        fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    screen                    = SDL_GetWindowSurface(window);
    player                    = SDL_LoadBMP("mudkip.bmp");
	missile                   = SDL_LoadBMP("missile.bmp");

    block                     = SDL_CreateRGBSurface(0,
                                                     blockbox.w,
                                                     blockbox.h,
                                                     24,
                                                     rmask,
                                                     gmask,
                                                     bmask,
                                                     amask);

    SDL_FillRect(screen,
                 NULL,
                 SDL_MapRGB(screen -> format, 0x00, 0x00, 0x00));

    SDL_FillRect(block,
                 NULL,
                 SDL_MapRGB(block   -> format, 0xFF, 0xFF, 0xFF));

    SDL_BlitSurface(player, &playerbox, screen, &playerpos);
    blockpos.x                = 0;
    blockpos.y                = 0;
    for (x = 0; x <= SCREEN_WIDTH; x+=blockpos.w)
    {
        blockpos.y            = 0;
        SDL_BlitSurface(block, NULL, screen, &blockpos);
        blockpos.y            = SCREEN_HEIGHT - blockbox.h;
        SDL_BlitSurface(block, NULL, screen, &blockpos);
        blockpos.x            = x;
    }

    blockpos.x                = 0;
    blockpos.y                = 0;
    for (x = 0; x <= SCREEN_HEIGHT; x+=blockpos.h)
    {
        blockpos.x            = 0;
        SDL_BlitSurface(block, NULL, screen, &blockpos);
        blockpos.x            = SCREEN_WIDTH - blockbox.w;
        SDL_BlitSurface(block, NULL, screen, &blockpos);
        blockpos.y            = x;
    }
    SDL_UpdateWindowSurface(window);

    // stuff

        while (quit          == 0)
        {
            SDL_PollEvent(&e);

            // button integration
            if (e.type       == SDL_JOYBUTTONDOWN)
            {
                if (e.jbutton.button == 1)
                {
                    p_go        = 10;
                }
            }
            else if (e.type  == SDL_JOYBUTTONUP)
            {
                p_go = 5;
            }

            // user clicks the 'X' on the window
            if (e.type       == SDL_QUIT)
            {
                quit          = 1;
                break;
            }
            else if (e.type  == SDL_JOYAXISMOTION && e.jaxis.value != 0)
            {
                if (e.jaxis.which == 0) // motion on controller 0
                {
                    if (e.jaxis.axis == 0) // on the X axis
                    {
                        fprintf(stdout, "%d\t%d\n", e.jaxis.axis, e.jaxis.value);
                        if (e.jaxis.value < 0) // left
                        {
                            playerpos.x  = playerpos.x - p_go;
                            frame        = 1;                            
                        }
                        else if (e.jaxis.value > 0) //right
                        {
                            playerpos.x  = playerpos.x + p_go;
                            frame        = 1;
                        }
                    }
                    else if (e.jaxis.axis == 1) // on the Y axis
                    {
                        fprintf(stdout, "%d\t%d\n", e.jaxis.axis, e.jaxis.value);
                        if (e.jaxis.value < 0) // up
                        {
                            playerpos.y = playerpos.y - p_go;
                            frame = 1;
                        }
                        else if (e.jaxis.value > 0) //down
                        {
                            playerpos.y = playerpos.y + p_go;
                            frame = 1;
                        }
                        
                    }
                }
            }
            else if (e.type   == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_UP:
                        playerpos.y     = playerpos.y - p_go;
                        frame     = 1;
						p_dir     = M_UP;
                        break;

                    case SDLK_DOWN:
                        playerpos.y     = playerpos.y + p_go;
                        frame     = 1;
						p_dir     = M_DN;
                        break;

                    case SDLK_LEFT:
                        playerpos.x     = playerpos.x - p_go;
                        frame     = 1;
						p_dir     = M_LF;
                        break;

                    case SDLK_RIGHT:
                        playerpos.x  = playerpos.x + p_go;
                        frame     = 1;
						p_dir     = M_RT;
                        break;

                    case SDLK_RETURN:
                        playerpos.x     = (SCREEN_WIDTH  / 2) - 35;
                        playerpos.y     = (SCREEN_HEIGHT / 2) - 43;
                        frame     = 1;
                        break;

					case SDLK_SPACE:
						if (mlist == NULL)
						{
							mlist  = (Missile *) malloc (sizeof(Missile));
							mlist -> surface = missile;
							mlist -> box  = missilebox;
							mlist -> pos  = missilepos;
							mlist -> next = NULL;
							mtmp          = mlist;
						}
						else
						{
							mtmp = mlist;
							while (mtmp -> next != NULL)
								mtmp = mtmp -> next;

							mtmp -> next = (Missile *) malloc (sizeof(Missile));
							mtmp         = mtmp -> next;
							mtmp -> surface = missile;
							mtmp -> box  = missilebox;
							mtmp -> pos  = missilepos;
							mtmp -> next = NULL;
						}
						mtmp -> pos.x = playerpos.x+(playerbox.w/2)-(mtmp -> box.w/2);
						mtmp -> pos.y = playerpos.y+(playerbox.h/2)-(mtmp -> box.h/2);
						mtmp -> m_dir = p_dir;
						mtmp -> box.x = 16 * mtmp -> m_dir;
						rfs = rand() % 9 + 1;
						mtmp -> m_go  = rfs;
						break;
						
                    case SDLK_ESCAPE:
                        quit = 1;
                        break;
                }
            }
            SDL_FillRect(screen,
                         NULL,
                         SDL_MapRGB(screen -> format, 0x00, 0x00, 0x00));

            if (playerpos.x < blockpos.w)
                playerpos.x = blockpos.w + 1;

            if (playerpos.x + playerpos.w > SCREEN_WIDTH - blockpos.w)
                playerpos.x = SCREEN_WIDTH - blockpos.w - playerpos.w;

            if (playerpos.y < blockpos.h)
                playerpos.y = blockpos.h + 1;

            if (playerpos.y + playerpos.h > SCREEN_HEIGHT - blockpos.h)
                playerpos.y = SCREEN_HEIGHT - blockpos.h - playerpos.h;

			mtmp         = mlist;
			while (mtmp != NULL)
			{
				if (mtmp -> pos.x < mtmp -> box.w)
					mtmp -> m_dir = -1;
				else if (mtmp -> pos.x > SCREEN_WIDTH - mtmp -> box.w)
					mtmp -> m_dir = -1;
				else if (mtmp -> pos.y < mtmp -> box.h)
					mtmp -> m_dir = -1;
				else if (mtmp -> pos.y > SCREEN_HEIGHT - mtmp -> box.h)
					mtmp -> m_dir = -1;

				if (mtmp -> m_dir != -1)
				{
					SDL_BlitSurface(mtmp -> surface, &(mtmp -> box), screen, &(mtmp -> pos));
	//				fprintf(stdout, "[missile] x: %d, y: %d\n", missilepos.x, missilepos.y);
				}

				if (mtmp -> m_dir == M_UP)
					mtmp -> pos.y = mtmp -> pos.y - mtmp -> m_go;
				else if (mtmp -> m_dir == M_DN)
					mtmp -> pos.y = mtmp -> pos.y + mtmp -> m_go;
				else if (mtmp -> m_dir == M_LF)
					mtmp -> pos.x = mtmp -> pos.x - mtmp -> m_go;
				else if (mtmp -> m_dir == M_RT)
					mtmp -> pos.x = mtmp -> pos.x + mtmp -> m_go;

				if (mtmp -> m_dir == -1) // remove ze dead missile
				{
					mtmp2 = mlist; // point at start of list
					if (mtmp2 != mtmp) // if mtmp is not the first one
					{
						while (mtmp2 -> next != mtmp)
						{
							mtmp2 = mtmp2 -> next;
						}
						mtmp2 -> next = mtmp -> next;
						mtmp  -> next = NULL;
						free(mtmp);
						mtmp = mtmp2 -> next;
					}
					else // mtmp and mtmp2 are the same
					{
						mlist = mlist -> next; // advance start of list
						mtmp  = mlist; // advance mtmp
						mtmp2 -> next = NULL; // disconnect dead missile
						free(mtmp2); // free it
						mtmp2 = NULL; // burn it in effigy
					}
				}

				if (mtmp != NULL)
					mtmp = mtmp -> next;
			}

            playerbox.x           = (((p_dir * 2) + frame) * playerbox.w);

            SDL_BlitSurface(player, &playerbox, screen, &playerpos);
            blockpos.x            = 0;
            blockpos.y            = 0;
            for (x = 0; x <= SCREEN_WIDTH; x+=blockpos.w)
            {
                blockpos.y        = 0;
                SDL_BlitSurface(block, NULL, screen, &blockpos);
                blockpos.y        = SCREEN_HEIGHT - blockbox.h;
                SDL_BlitSurface(block, NULL, screen, &blockpos);
                blockpos.x        = x;
            }

            blockpos.x            = 0;
            blockpos.y            = 0;
            for (x = 0; x <= SCREEN_HEIGHT; x+=blockpos.h)
            {
                blockpos.x        = 0;
                SDL_BlitSurface(block, NULL, screen, &blockpos);
                blockpos.x        = SCREEN_WIDTH - blockbox.w;
                SDL_BlitSurface(block, NULL, screen, &blockpos);
                blockpos.y        = x;
            }
            SDL_UpdateWindowSurface(window);
			frame                 = 0;
        }

    for (x = 0; x < num_pads; x++)
    {
        SDL_JoystickClose(*(pad+x));
        *(pad+x)                  = NULL;
    }

    // at the end
    SDL_DestroyWindow(window);
    SDL_Quit();

    return(0);
}
