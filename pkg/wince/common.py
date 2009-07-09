
# SDL library files that need to be installed:

LIBRARIES = [ "SDL.dll", "SDL_mixer.dll", "libSDL_net-1-2-0.dll" ]

def add_libraries(dir, files):
    for lib in LIBRARIES:
        files[dir + lib] = lib

