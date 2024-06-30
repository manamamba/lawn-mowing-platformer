# Lawn Mowing Platformer
#### Video Demo: 
#### Description: My first game, a physics-based platformer with procedural spawning, moving platforms and localized gravity, made in Unreal Engine 5.

<p>Hello, my name is Cody and for my CS50 final project I made a platforming demo with Unreal Engine 5 and C++. 
From start to finish, this project took around 2 months to complete, or a little over 4 if you count the time I spent beforehand on these tutorials:</p>

[`Learn C++`](https://www.learncpp.com/)
[`Unreal Engine 5 C++ Developer: Learn C++ & Make Video Games`](https://www.udemy.com/course/unrealcourse/)
[Blender Guru’s Blender 4.0 Beginner Donut Tutorial](https://www.youtube.com/playlist?list=PLjEaoINr3zgEPv5y--4MKpciLaoQYZB1Z)

<p>The concepts used for building my lawn mower’s physics-based movement came from this video:</p>

[Space Dust Racing UE4 Arcade Vehicle Physics Tour](https://www.youtube.com/watch?v=LG1CtlFRmpU)

<p>All of the code written for this project relates to classes derived from the Actor class in Unreal’s codebase, 
usually with a focus on functions called within the constructor, and the virtual functions BeginPlay() and Tick(). 
Each class has a blueprint counterpart in Unreal to visually represent how it is constructed. 
When the game is running and an instance of an Actor is instantiated in the level, a call to their BeginPlay() function is made, 
then a call to their Tick() function each frame.</p>

<strong>MowerB</strong>

<p>The lawn mower was the first class that I developed for this project, which is the Actor controlled by the player. 
In its constructor, I attach many components and configure their properties. Additionally, I add two event listeners, 
one to play a crash sound when the primitive component used for physics is hit by another object, and the other to start a particle emitter 
when grass is overlapped with the mower.</p>

<p>In BeginPlay(), I override the primitive component’s mass and lower it to make it harder for the mower to get stuck upside down. 
Then I store some information about the state of the primitive component for later and set the camera’s initial rotation. 
Lastly, I set up enhanced input for the controls.</p>

<p>Before defining the Tick() function, I define a few public member functions that I will call from other classes for localized gravity and checkpoints.</p>

<p>In Tick(), the first and last function calls pertain to calculating the time it takes for the game to process the function each frame. 
Following that, I check if the mower has fallen too far and needs to be respawned, then I store information about its location and rotation to use later. 
Next, I use the location of the mower from the previous frame and compare it to the current frame to determine the direction and speed of the mower for other functions. 
Then I update the camera’s rotation based on player input and the current rotation of the mower.</p>


<p>The next series of functions pertain to simulating wheel suspension and making the mower hover. Here I use const member variables to represent 
vectors in the mower’s local space, then I transform those vectors relative to the mower in world space and use them to start raycasts that look for the ground. 
If ground is hit, force is applied to the primitive, based on the length of the raycast when ground is hit, in the direction of the hit normal where the raycast began. 
Then, some amount of friction is stored in arrays that are used later to make the hover stable.</p>

<p>After establishing the hover and suspension, I update some boolean logic for movement, based on the player’s input this frame and last frame. 
Next, using this information and the player’s input, I update member variables pertaining to acceleration, drifting, jumping, pitching and rolling, 
then apply forces to the primitive based on them. Lastly for physics, friction is applied to the mower based on whether or not 
it is grounded and if the brakes are engaged.</p>

<p>Afterwards, a series of functions are called to update the animation of the mower’s wheels, spinning blade and vibration. 
Next, a member variable pertaining to the particle emitter is updated, which limits the frequency of consecutive activations. 
Then, the pitch of the mower’s hum is regulated based on its acceleration speed, and variables are updated to limit the frequency of crash and cutting audio.</p>

<p>For the remaining functions called in Tick(), they pertain to visualizing forces and printing movement information to the console for analysis. 
Lastly, member variables storing information about friction and player input are reset to prepare for the next frame.</p>

<strong>GrassE and GrassSpawnerD</strong> 

<p>When new grass is spawned in the level, it only consists of a root component before a static mesh component is attached and configured. 
In this instant, an event listener is also set up, so that the Actor is destroyed when it comes into contact with the lawn mower. 
Then, each frame the blade of grass will attempt to spawn another blade of grass if it is ready. When it is ready, a raycast is sent towards the ground. 
If ground is hit, a sweep is done at the location to check if there is an overlap with a collider that allows new grass to spawn, 
and check if there is an overlap with other grass. If there is an overlap with the collider and not other grass, then the position and length of the raycast 
will be used to determine the angle of the new grass to spawn. After the grass has completed this process six times, 
it will stop calling its Tick() function each frame.</p>

<p>The grass spawner initiates the first spawn of grass inside of a spawn enabling collider. In the constructor, an event listener is set up that will 
activate the spawner when the lawn mower overlaps its collider. Each frame, it will check the number of grass that originated from it versus the number 
of those that have been destroyed. This information is used to activate moving platforms, set new checkpoints for the mower, or restart the game when all 
of its grass is destroyed.</p>

<strong>MovingPlatformA</strong>

<p>The moving platform class consists of blueprint exposed member variables to set the movement, rotation, speed, and method of activation for an instance 
of this class in the level. Based on the values set, the platform will move and/or rotate each frame at the specified speed.</p>

<strong>PlanetoidA</strong>

<p>When the game begins, an instance of this class uses a pointer to the lawn mower to access its primitive component and mass to set its capture force. 
Then, event listeners are set up to negate the normal gravity being applied to the mower and apply capture force when the mower is overlapping its collider. 
The direction of the capture force is calculated by subtracting the location of this Actor from the location of the lawn mower, then normalizing the resulting vector.</p>

<strong>MowerGameModeA</strong>

<p>This class is used to loop the demo’s song in the background.</p>

<strong>MowerPlaerControllerA</strong>

<p>This Actor’s functions are called by player input from the lawn mower class to navigate the pause menu. Based on these inputs, enum member variables 
are updated to represent directional movement in the menu. When the game begins, an instance of the menu is created, then all of its text boxes are stored in an array
that is accessed later to highlight the selected option’s text.</p>


<strong>Setup Instructions</strong>

<p>This project was made using Unreal Engine 5.3.2 and Visual Studio 2022 as the source code editor, feel free to explore the project yourself in Unreal:</p>

[Unreal and Visual Studio Setup Guide](https://www.youtube.com/watch?v=HQDskHVw1to)

<ol>
    <li>Extract files into a project folder</li>
    <li>Right-click the uproject file and Generate Visual Studio project files</li>
    <li>Open the solution file, then build and launch Unreal using Ctrl + F5</li>
</ol>

