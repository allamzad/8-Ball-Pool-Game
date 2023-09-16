# 8-Ball-Pool-Game

![pool banner](https://www.ultimatebattle.in/wp-content/uploads/2017/06/8-ball-pool-banner-min-1349x310.jpg)

Eight-ball 🎱 (also spelled 8-ball or eightball) is a discipline of pool played on a billiard table with six pockets, cue sticks, and sixteen billiard balls (a cue ball and fifteen object balls). 

This project is a game following the rules and gameplay of Eight-ball built in C using Emscripten and SDL2 technologies. The project encompasses a wide array of functionalities including AI-driven enemy bots, a robust physics engine implementing forces and motion, diverse game modes, audio integration, and more, in order to create an enjoyable experience for the user.

* AI-driven enemy bots 🤖: The game features a two-player mode where players can play against eachother, as well as an AI mode (available in easy and medium difficulties). The AI bot calculates if a ball of their type can be hit into a pocket, and based on the difficulty, randomly aims the cuestick within the direction of the cue ball with some margin of error.

* Diverse game modes 🎮: The game features a Chaos gamemode where the impulse of one ball creates an impulse in the next, increasing the acceleration of the balls on the table (the balls do not move based on collisions, but forces are transferred from one ball to another). Deathmatch mode is a mode where if a player does not hit in a ball during their turn, they will lose (except for turns where stripes/solids are not yet decided)

* Powerups 🪙: If enabled, hitting a powerup with the cueball will allow you an additional turn!
  
* Physics Engine :atom:: For a realistic feel, this pool game includes newtonian gravity, elastic forces, drag forces, and collision physics! (check library/forces.c and library/collisions.c)

* Audio/Graphics/Text integration 🔉🖼️🔡: During the game, background music will be played and sound effects will player upon certain triggers (ball collisions, balls going into pockets, etc) using SDL2_mixer. Sprite graphics were also implemented for the pool balls, cuestick, pool table, and more using SDL2_image. Lastly, text was overlayed for the menu and in-game using SDL2_ttf.

* User-input/Mouse-input 🖱️: Using SDL2 Mouse packages, the game detects keystrokes and mouse inputs, particularly useful in selecting menu options and interacting with the cuestick.

* And more 🎱: The game has a functional menu, instructions, a turn system, winning conditions, displays balls hit into pockets, shows the cueball trajectory, and includes all the features one would expect to see in a pool game.


## How to run the game:
The game was designed to be run on Caltech's labrodoodle remote connection where each user would ssh to USERNAME@labrodoodle.caltech.edu. Cloning the repository and running `make all` in the terminal would then compile the files and make the game available at http://labradoodle.caltech.edu:####/bin/pool.html where #### is a 4-digit code unique to the user. Because the game currently does not have the capability to be ran by users outside of Caltech, images from the game are shown below.

## Gameplay:






