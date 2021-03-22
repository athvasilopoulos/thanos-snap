# Thanos Snap
 
The goal of this project is to recreate the well known "Thanos Snap" special effect as seen in the Marvel movies and integrate it into a minigame.<br>
The project is currently in alpha version, with performance and visual upgrades on the works.

![](fig/kill-effect.gif)
## Build instructions

The project can be built using CMake. The procedure is the following:
- Download [CMake](https://cmake.org/download/)
- Clone the repository
- Launch CMake. In the first line navigate to the project directory and in the second line enter where you want all the compiler's stuff to live, for example a new folder inside the directory named "build".
- Click configure and choose your preferred compiler/generator (the project was developed using Visual Studio) 
- Click on generate to create the project

## Game description
The game consists of the following basic mechanics:
- FPS-like controls, with a Thanos glove instead of a weapon
- Targeting and killing targets using the glove and the finger snap animation
- The dead target disappears gradually with its pieces starting to float away and with small spheres jumping out from the inside
- The targets start moving towards the user after the first kill and the user loses if he gets touched by a target
